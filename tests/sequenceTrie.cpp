#include "gtest/gtest.h"
#include <cstdint>
#include <jaffarCommon/sequenceTrie.hpp>
#include <vector>

using jaffarCommon::sequenceTrie::SequenceTrie;
using Trie = SequenceTrie<uint16_t>;

// Basic functionality: extend builds a path, reconstruct returns it root-first, depth is the path length.
TEST(sequenceTrie, extendAndReconstruct)
{
  Trie       t(/*numShards=*/1, /*chunkSizeLog2=*/8);
  const auto a = t.extend(Trie::ROOT, 3, 0);
  const auto b = t.extend(a, 7, 0);
  const auto c = t.extend(b, 9, 0);

  std::vector<uint16_t> seq;
  t.reconstruct(c, seq);
  ASSERT_EQ(seq.size(), 3u);
  EXPECT_EQ(seq[0], 3);
  EXPECT_EQ(seq[1], 7);
  EXPECT_EQ(seq[2], 9);
  EXPECT_EQ(t.getDepth(c), 3u);
  EXPECT_FALSE(t.isExhausted());

  t.release(c, 0);
}

// Capacity guard: at the hard node-storage ceiling, extend() must SOFT-FAIL -- return NONE and latch
// isExhausted() -- rather than throw. A throw here would escape the search's parallel region and call
// std::terminate ("terminate called recursively" as every worker throws at once). Tiny chunks make the
// fixed MAX_CHUNKS ceiling reachable in tens of thousands of nodes.
TEST(sequenceTrie, capacityGuardSoftFails)
{
  Trie t(/*numShards=*/1, /*chunkSizeLog2=*/1); // 2 nodes per chunk

  Trie::nodeId_t node    = Trie::ROOT;
  size_t         ok      = 0;
  bool           sawNone = false;
  for (size_t i = 0; i < (1u << 20); i++)
  {
    const Trie::nodeId_t next = t.extend(node, (uint16_t)(i & 0xFFFF), 0);
    if (next == Trie::NONE)
    {
      sawNone = true;
      break;
    }
    node = next;
    ok++;
  }

  EXPECT_TRUE(sawNone);         // hit the ceiling without throwing
  EXPECT_TRUE(t.isExhausted()); // and latched the exhausted flag
  EXPECT_GT(ok, 1000u);         // sanity: it stored a meaningful number of nodes first

  // The trie must remain usable after exhaustion (no corruption): reconstruct the deepest live path, and
  // further extends keep returning NONE instead of crashing.
  std::vector<uint16_t> seq;
  t.reconstruct(node, seq);
  EXPECT_EQ(seq.size(), ok);
  EXPECT_EQ(t.extend(node, 1, 0), Trie::NONE);
}
