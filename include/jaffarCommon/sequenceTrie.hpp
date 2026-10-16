#pragma once

/**
 * @file sequenceTrie.hpp
 * @brief A concurrent, reference-counted trie that stores many sequences compactly by sharing their
 *        common prefixes. Each stored sequence is identified by a small integer node handle; the full
 *        sequence is recovered by walking parent links back to the root.
 *
 * Motivation: a best-first search keeps a large frontier of states, each of which needs to remember the
 * path (sequence of moves/inputs) that produced it. Storing the whole path in every state duplicates the
 * long prefixes that sibling states share. This trie stores each path once: a state keeps only an 8-byte
 * @c nodeId_t, and `extend(parent, element)` adds one element on top of an existing path. Nodes are
 * reference counted so the structure stays bounded by the paths of *live* states: when the last holder of
 * a leaf releases it, the leaf (and any ancestors that become childless and unreferenced) are recycled.
 *
 * Concurrency: `extend`, `acquire`, `release` and `reconstruct` are all safe to call concurrently, with
 * one rule: each free-list *shard* belongs to one thread at a time (pass each thread its own shard id;
 * shard 0 is the default). Node storage is a fixed array of lazily-allocated fixed-size chunks (handles
 * are stable indices that never move). Freed nodes go to the releasing thread's shard, a plain stack
 * with no atomics; fresh ids are handed to each shard in blocks, so the shared id counter is touched once
 * per block; only the rare allocation of a brand-new chunk takes a mutex. The reference-count invariant
 * is the usual one: you may only `extend` from, or `acquire`, a node you already hold a reference to, so
 * a node at refcount zero can never be revived.
 *
 * Memory: a node takes 16 bytes (8-byte parent id, 2-byte element, 2-byte reference count, padding). The
 * node storage is capped at `getMaxMemoryBytes` (by default the full chunk table, 24576 chunks; lower
 * it with `setMaxMemoryBytes`); past the cap, `extend` returns `NONE`.
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
  /// 64-bit so the node-id space (and thus the live-node ceiling) exceeds 2^32: a deep, many-state
  /// search (e.g. a full-race input trie) accumulates more than 4.29 B path nodes. Widening this to
  /// uint64 grows Node from 8 to 16 bytes (the `parent` field), so the trie costs ~2x RAM per node.
  using nodeId_t = uint64_t;

  /// @brief The empty sequence. Always valid, never recycled; the base of every path.
  static constexpr nodeId_t ROOT = 0;

  /// @brief Sentinel for "no node" (also the free-list terminator).
  static constexpr nodeId_t NONE = 0xFFFFFFFFFFFFFFFFull;

  /**
   * @brief Constructs an empty trie containing only @ref ROOT.
   * @param numShards Number of independent free-list shards used to reduce contention (default 1).
   * @param chunkSizeLog2 Log2 of the number of nodes per storage chunk (default 2^20 nodes/chunk).
   */
  explicit SequenceTrie(uint32_t numShards = 1, uint32_t chunkSizeLog2 = 20)
      : _chunkBits(chunkSizeLog2), _chunkSize(1u << chunkSizeLog2), _chunkMask((1u << chunkSizeLog2) - 1), _shards(numShards == 0 ? 1 : numShards)
  {
    for (auto& c : _chunks) c.store(nullptr, std::memory_order_relaxed);
    for (auto& s : _shards) s.head = NONE;
    _bump.store(0, std::memory_order_relaxed);
    _maxNodes = MAX_CHUNKS * (uint64_t)_chunkSize;

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
   * @param shard Free-list shard to allocate the new node from (default 0).
   * @return The new node, carrying one reference owned by the caller (balance with @ref release).
   * @details Thread-safe. The new node adds a child link to @p parent, which keeps @p parent alive for
   * at least as long as the new node.
   */
  nodeId_t extend(nodeId_t parent, Element element, uint32_t shard = 0)
  {
    // A cursor left at NONE (a previous extend failed at the ceiling) stays NONE: there is no path to extend
    if (parent == NONE) return NONE;
    const nodeId_t id = allocNode(shard);
    if (id == NONE) return NONE; // trie exhausted (hard ceiling): propagate "couldn't store" to the caller
    Node& n   = node(id);
    n.parent  = parent;
    n.element = element;
    n.refCount.store(1, std::memory_order_relaxed); // the caller's reference

    // Account for the new child edge on the parent (kept alive by its children).
    incRef(node(parent));
    return id;
  }

  /**
   * @brief Adds one reference to @p id (when a new holder starts referencing it). Thread-safe.
   * @param id The node to add a reference to.
   */
  void acquire(nodeId_t id) { incRef(node(id)); }

  /**
   * @brief Drops one reference from @p id. Thread-safe.
   * @param id The node to drop a reference from.
   * @param shard Free-list shard that recycled node slots are returned to (default 0).
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
  template <typename OutElem = Element>
  void reconstruct(nodeId_t id, std::vector<OutElem>& out) const
  {
    out.clear();
    for (nodeId_t cur = id; cur != ROOT && cur != NONE;)
    {
      const Node& n = node(cur);
      out.push_back(static_cast<OutElem>(n.element)); // widen the 16-bit stored element to the caller's type
      cur = n.parent;
    }
    std::reverse(out.begin(), out.end());
  }

  /**
   * @brief Number of elements from ROOT to @p id (its depth in the trie).
   * @param id The node whose depth is measured.
   * @return The number of elements on the path from ROOT to @p id.
   */
  size_t getDepth(nodeId_t id) const
  {
    size_t d = 0;
    for (nodeId_t cur = id; cur != ROOT && cur != NONE; cur = node(cur).parent) d++;
    return d;
  }

  /**
   * @brief Total node slots ever handed out as fresh ids (high-water of simultaneously-live nodes, since
   * freed nodes are recycled before fresh ids are used; it includes the unused rest of each shard's
   * current block of fresh ids). A good proxy for the trie's resident memory.
   * @return The number of node slots ever taken from the fresh-id counter.
   */
  size_t getAllocatedNodeCount() const { return std::min<uint64_t>(_bump.load(std::memory_order_relaxed), _maxNodes); }

  /**
   * @brief Approximate resident memory of the trie's node storage, in bytes.
   * @return The approximate node-storage footprint in bytes.
   */
  size_t getApproxMemoryBytes() const { return getAllocatedNodeCount() * sizeof(Node); }

  /**
   * @brief Hard upper bound on the trie's node storage, in bytes: the most it can ever occupy before
   *        node allocation throws "exceeded maximum capacity". The live trie is otherwise unbounded (it
   *        grows ~ live-states x depth), so callers that must reserve RAM up front (e.g. the engine's
   *        memory guard) use this fixed ceiling. = MAX_CHUNKS * nodes-per-chunk * sizeof(Node).
   * @return The hard upper bound on the trie's node-storage footprint, in bytes.
   */
  size_t getMaxMemoryBytes() const { return _maxNodes * sizeof(Node); }

  /**
   * @brief Lowers the cap on the trie's node storage (by default the whole chunk table: 24576 chunks).
   * @param bytes The most node storage the trie may take; rounded down to whole chunks, at least one chunk
   *              and at most the default.
   * @note Call before the trie is used concurrently.
   */
  void setMaxMemoryBytes(const size_t bytes)
  {
    const size_t chunkBytes = (size_t)_chunkSize * sizeof(Node);
    const size_t chunks     = std::clamp<size_t>(bytes / chunkBytes, 1, MAX_CHUNKS);
    _maxNodes               = chunks * (uint64_t)_chunkSize;
  }

  /// @brief True once node allocation has hit the hard capacity ceiling (@ref getMaxMemoryBytes). Latched
  /// and never cleared. After this, @ref extend returns @ref NONE ("couldn't store") instead of throwing,
  /// so a caller (e.g. a search driver) can poll this and stop the run gracefully rather than terminate.
  /// @return Whether node allocation has hit the capacity ceiling.
  bool isExhausted() const { return _exhausted.load(std::memory_order_relaxed); }

private:
  struct Node
  {
    nodeId_t              parent;   ///< Parent node id when live; next-free id when on the free list.
    Element               element;  ///< The element on the edge from parent to this node.
    std::atomic<uint16_t> refCount; ///< (# external holders) + (# child edges). Zero => recyclable. 16-bit:
                                    ///< this is a node's local out-degree + holders (never its subtree
                                    ///< size), bounded by the input-set size; @ref incRef guards overflow.
  };

  // Fixed size of the chunk-pointer array, so it never reallocates (handles stay valid lock-free). With the
  // default 2^20 nodes per chunk and 16 B per node this allows up to ~384 GiB of nodes; setMaxMemoryBytes()
  // lowers the actual cap. With 64-bit node ids the count is not 2^32-bound.
  static constexpr size_t MAX_CHUNKS = 24576;

  // Fresh ids are handed to a shard this many at a time, so the shared counter is touched once per block.
  static constexpr uint64_t FRESH_BLOCK = 256;

  __attribute__((always_inline)) Node& node(nodeId_t id) const { return _chunks[id >> _chunkBits].load(std::memory_order_acquire)[id & _chunkMask]; }

  /// @brief Adds one reference, guarding the 16-bit refCount width. A node's refCount is its local
  /// out-degree plus the few holders pointing at it (NOT its subtree size), so it is bounded by the
  /// input-set size and never approaches 65535 in practice; this guard turns a would-be silent
  /// wraparound (which would prematurely free a live node) into a hard error.
  __attribute__((always_inline)) void incRef(Node& n) const
  {
    if (n.refCount.fetch_add(1, std::memory_order_relaxed) == UINT16_MAX)
      JAFFAR_THROW_RUNTIME("SequenceTrie node refCount overflowed 16 bits (out-degree + holders > %u); the "
                           "input set is too large for the 16-bit-refCount trie node.",
                           (unsigned)UINT16_MAX);
  }

  // Obtain a node slot: reuse one recycled into this shard's (thread-private) free list if available,
  // otherwise take the next id of the shard's current block of fresh ids, getting a new block from the
  // shared counter when it runs out. Neither the free list nor the block needs atomics because a given
  // shard is only ever touched by its owning thread; the shared counter is touched once per block.
  nodeId_t allocNode(uint32_t shard)
  {
    FreeShard& fs = _shards[shard];
    if (fs.head != NONE)
    {
      const nodeId_t top = fs.head;
      fs.head            = node(top).parent; // free-list "next" stored in parent
      return top;
    }

    // Free list empty: take a fresh id, faulting in its chunk on first use.
    if (fs.freshNext == fs.freshEnd)
    {
      // Capacity reached: do NOT throw here. This runs inside the parallel search region across many worker
      // threads; a throw escaping an OpenMP region calls std::terminate. Instead, latch an exhausted flag and
      // return NONE -- callers (extend) propagate NONE as "couldn't store", and the driver polls isExhausted()
      // to stop the run gracefully. Once latched, the shared counter is no longer touched.
      if (_exhausted.load(std::memory_order_relaxed)) return NONE;
      const uint64_t base = _bump.fetch_add(FRESH_BLOCK, std::memory_order_relaxed);
      if (base >= _maxNodes)
      {
        _exhausted.store(true, std::memory_order_relaxed);
        return NONE;
      }
      fs.freshNext = base;
      fs.freshEnd  = std::min(base + FRESH_BLOCK, _maxNodes);
    }
    const nodeId_t id    = fs.freshNext++;
    const size_t   chunk = id >> _chunkBits;
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

  /// @brief A per-shard recycled-node free list and block of fresh ids. Cache-line aligned so adjacent
  /// shards never share a line; touched only by the shard's owning thread, so it needs no atomics.
  struct alignas(64) FreeShard
  {
    nodeId_t head      = NONE; ///< Top of the recycled-node stack.
    uint64_t freshNext = 0;    ///< Next unused fresh id of the shard's current block.
    uint64_t freshEnd  = 0;    ///< End (exclusive) of the shard's current block of fresh ids.
  };

  const uint32_t _chunkBits;
  const uint32_t _chunkSize;
  const uint32_t _chunkMask;

  mutable std::array<std::atomic<Node*>, MAX_CHUNKS> _chunks;
  uint64_t                                           _maxNodes = 0; ///< Node-id ceiling (see @ref setMaxMemoryBytes).
  alignas(64) std::atomic<uint64_t> _bump;                          ///< Start of the next block of fresh node ids (on its own cache line).
  std::vector<FreeShard> _shards;
  alignas(64) std::mutex _growMutex;
  std::atomic<bool> _exhausted{false}; ///< Latched when the node-id ceiling is hit (see @ref isExhausted).
};

} // namespace sequenceTrie

} // namespace jaffarCommon
