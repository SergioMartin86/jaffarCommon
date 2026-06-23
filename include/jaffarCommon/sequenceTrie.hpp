#pragma once

/**
 * @file sequenceTrie.hpp
 * @brief A concurrent, reference-counted trie that stores many sequences compactly by sharing their
 *        common prefixes. Each stored sequence is identified by a small integer node handle; the full
 *        sequence is recovered by walking parent links back to the root.
 *
 * Motivation: a best-first search keeps a large frontier of states, each of which needs to remember the
 * path (sequence of moves/inputs) that produced it. Storing the whole path in every state duplicates the
 * long prefixes that sibling states share. This trie stores each path once: a state keeps only a 4-byte
 * @ref nodeId_t, and `extend(parent, element)` adds one element on top of an existing path. Nodes are
 * reference counted so the structure stays bounded by the paths of *live* states: when the last holder of
 * a leaf releases it, the leaf (and any ancestors that become childless and unreferenced) are recycled.
 *
 * Concurrency: `extend`, `acquire`, `release` and `reconstruct` are all safe to call concurrently. Node
 * storage is a fixed array of lazily-allocated fixed-size chunks (handles are stable indices that never
 * move), recycled through a lock-free (ABA-tagged) free list; only the rare allocation of a brand-new
 * chunk takes a mutex. The reference-count invariant is the usual one: you may only `extend` from, or
 * `acquire`, a node you already hold a reference to, so a node at refcount zero can never be revived.
 */

#include "exceptions.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

namespace jaffarCommon
{

namespace sequenceTrie
{

/**
 * @brief A concurrent reference-counted prefix trie over sequences of @p Element.
 * @tparam Element Trivially-copyable element type stored on each edge (e.g. an input index).
 */
template <typename Element>
class SequenceTrie
{
public:
  /// @brief Handle identifying a node (a stored sequence). Stable for the node's lifetime.
  using nodeId_t = uint32_t;

  /// @brief The empty sequence. Always valid, never recycled; the base of every path.
  static constexpr nodeId_t ROOT = 0;

  /// @brief Sentinel for "no node" (also the free-list terminator).
  static constexpr nodeId_t NONE = 0xFFFFFFFFu;

  /**
   * @brief Constructs an empty trie containing only @ref ROOT.
   * @param chunkSizeLog2 Log2 of the number of nodes per storage chunk (default 2^20 nodes/chunk).
   */
  explicit SequenceTrie(uint32_t numShards = 1, uint32_t chunkSizeLog2 = 20)
      : _chunkBits(chunkSizeLog2), _chunkSize(1u << chunkSizeLog2), _chunkMask((1u << chunkSizeLog2) - 1), _shards(numShards == 0 ? 1 : numShards)
  {
    for (auto& c : _chunks) c.store(nullptr, std::memory_order_relaxed);
    for (auto& s : _shards) s.head = NONE;
    _bump.store(0, std::memory_order_relaxed);

    // Allocate ROOT (id 0). It carries a permanent self-reference so it is never recycled.
    const nodeId_t root = allocNode(0);
    Node&          r    = node(root);
    r.parent            = NONE;
    r.element           = Element{};
    r.refCount.store(1, std::memory_order_relaxed);
  }

  ~SequenceTrie()
  {
    for (auto& c : _chunks)
    {
      Node* p = c.load(std::memory_order_relaxed);
      delete[] p;
    }
  }

  SequenceTrie(const SequenceTrie&)            = delete;
  SequenceTrie& operator=(const SequenceTrie&) = delete;

  /**
   * @brief Appends @p element after @p parent, returning a handle to the new sequence.
   * @param parent A node the caller holds a reference to (e.g. ROOT, or a state's stored node).
   * @param element The element to append.
   * @return The new node, carrying one reference owned by the caller (balance with @ref release).
   * @details Thread-safe. The new node adds a child link to @p parent, which keeps @p parent alive for
   * at least as long as the new node.
   */
  nodeId_t extend(nodeId_t parent, Element element, uint32_t shard = 0)
  {
    const nodeId_t id = allocNode(shard);
    Node&          n  = node(id);
    n.parent          = parent;
    n.element         = element;
    n.refCount.store(1, std::memory_order_relaxed); // the caller's reference

    // Account for the new child edge on the parent (kept alive by its children).
    node(parent).refCount.fetch_add(1, std::memory_order_relaxed);
    return id;
  }

  /// @brief Adds one reference to @p id (when a new holder starts referencing it). Thread-safe.
  void acquire(nodeId_t id) { node(id).refCount.fetch_add(1, std::memory_order_relaxed); }

  /**
   * @brief Drops one reference from @p id. Thread-safe.
   * @details When a node's reference count reaches zero (no holders and no children) it is recycled and
   * the drop cascades to its parent (whose child edge just disappeared). ROOT's permanent reference keeps
   * the cascade from ever freeing it.
   */
  void release(nodeId_t id, uint32_t shard = 0)
  {
    while (id != NONE)
    {
      Node& n = node(id);
      // release-acquire so a thread that observes the count hit zero sees all prior writes to the node.
      if (n.refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) break; // still referenced -> stop

      const nodeId_t parent = n.parent; // read before the slot is handed to the free list
      freeNode(id, shard);
      id = parent; // the freed node was a child of `parent`; drop that child edge too
    }
  }

  /**
   * @brief Reconstructs the full sequence from @ref ROOT to @p id, in root-first order.
   * @param id  A node the caller keeps alive for the duration of the call.
   * @param out Cleared and filled with the sequence (ROOT contributes no element).
   */
  void reconstruct(nodeId_t id, std::vector<Element>& out) const
  {
    out.clear();
    for (nodeId_t cur = id; cur != ROOT && cur != NONE;)
    {
      const Node& n = node(cur);
      out.push_back(n.element);
      cur = n.parent;
    }
    std::reverse(out.begin(), out.end());
  }

  /// @brief Number of elements from ROOT to @p id (its depth in the trie).
  size_t getDepth(nodeId_t id) const
  {
    size_t d = 0;
    for (nodeId_t cur = id; cur != ROOT && cur != NONE; cur = node(cur).parent) d++;
    return d;
  }

  /// @brief Total node slots ever bump-allocated (high-water of simultaneously-live nodes, since freed
  /// nodes are recycled before fresh ids are bumped). A good proxy for the trie's resident memory.
  size_t getAllocatedNodeCount() const { return _bump.load(std::memory_order_relaxed); }

  /// @brief Approximate resident memory of the trie's node storage, in bytes.
  size_t getApproxMemoryBytes() const { return getAllocatedNodeCount() * sizeof(Node); }

private:
  struct Node
  {
    uint32_t              parent;   ///< Parent node id when live; next-free id when on the free list.
    Element               element;  ///< The element on the edge from parent to this node.
    std::atomic<uint32_t> refCount; ///< (# external holders) + (# child edges). Zero => recyclable.
  };

  // Fixed cap on chunks so the chunk-pointer array never reallocates (handles stay valid lock-free).
  static constexpr size_t MAX_CHUNKS = 4096;

  __attribute__((always_inline)) Node& node(nodeId_t id) const { return _chunks[id >> _chunkBits].load(std::memory_order_acquire)[id & _chunkMask]; }

  // Obtain a node slot: reuse one recycled into this shard's (thread-private) free list if available,
  // otherwise bump-allocate a fresh id. The per-shard free list needs no atomics because a given shard
  // is only ever touched by its owning thread -- removing the single-free-list contention that otherwise
  // serializes all workers. Only the rare fresh-id bump (free list empty) touches a shared atomic.
  nodeId_t allocNode(uint32_t shard)
  {
    FreeShard& fs = _shards[shard];
    if (fs.head != NONE)
    {
      const nodeId_t top = fs.head;
      fs.head            = node(top).parent; // free-list "next" stored in parent
      return top;
    }

    // Free list empty: bump-allocate a fresh id, faulting in its chunk on first use.
    const nodeId_t id    = (nodeId_t)_bump.fetch_add(1, std::memory_order_relaxed);
    const size_t   chunk = id >> _chunkBits;
    if (chunk >= MAX_CHUNKS) JAFFAR_THROW_RUNTIME("SequenceTrie exceeded its maximum capacity (%zu chunks)", MAX_CHUNKS);
    if (_chunks[chunk].load(std::memory_order_acquire) == nullptr) ensureChunk(chunk);
    return id;
  }

  void freeNode(nodeId_t id, uint32_t shard)
  {
    FreeShard& fs   = _shards[shard];
    node(id).parent = fs.head; // push onto this shard's private stack
    fs.head         = id;
  }

  void ensureChunk(size_t chunk)
  {
    std::lock_guard<std::mutex> lock(_growMutex);
    if (_chunks[chunk].load(std::memory_order_relaxed) != nullptr) return; // another thread won the race
    Node* p = new Node[_chunkSize];
    _chunks[chunk].store(p, std::memory_order_release);
  }

  /// @brief A per-shard recycled-node free list. Cache-line aligned so adjacent shards never share a
  /// line; touched only by the shard's owning thread, so its head needs no atomics.
  struct alignas(64) FreeShard
  {
    nodeId_t head = NONE;
  };

  const uint32_t _chunkBits;
  const uint32_t _chunkSize;
  const uint32_t _chunkMask;

  mutable std::array<std::atomic<Node*>, MAX_CHUNKS> _chunks;
  std::atomic<uint64_t>                              _bump; ///< Next fresh node id (only touched when a shard's free list is empty).
  std::vector<FreeShard>                             _shards;
  std::mutex                                         _growMutex;
};

} // namespace sequenceTrie

} // namespace jaffarCommon
