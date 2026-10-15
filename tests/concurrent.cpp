#include <atomic>
#include "gtest/gtest.h"
#include <jaffarCommon/concurrent.hpp>
#include <omp.h>
#include <vector>

using namespace jaffarCommon::concurrent;

TEST(concurrent, deque)
{
  Deque<int> d;

  ASSERT_NO_THROW(d.getInternalStorage());

  ASSERT_EQ(d.wasSize(), 0);
  ASSERT_NO_THROW(d.push_back_no_lock(1));
  ASSERT_EQ(d.wasSize(), 1);
  ASSERT_NO_THROW(d.push_front_no_lock(2));
  ASSERT_EQ(d.wasSize(), 2);
  ASSERT_EQ(d.front(), 2);
  ASSERT_EQ(d.back(), 1);
  
  int val = -1;
  
  ASSERT_TRUE(d.pop_front_get(val));
  ASSERT_EQ(val, 2);
  ASSERT_EQ(d.wasSize(), 1);
  
  ASSERT_TRUE(d.pop_front_get(val));
  ASSERT_EQ(val, 1);
  ASSERT_EQ(d.wasSize(), 0);
  
  ASSERT_FALSE(d.pop_front_get(val));

  ASSERT_NO_THROW(d.push_back_no_lock(1));
  ASSERT_NO_THROW(d.push_front_no_lock(2));
  ASSERT_EQ(d.wasSize(), 2);
  
  ASSERT_TRUE(d.pop_back_get(val));
  ASSERT_EQ(val, 1);
  ASSERT_EQ(d.wasSize(), 1);

  ASSERT_TRUE(d.pop_back_get(val));
  ASSERT_EQ(val, 2);
  ASSERT_EQ(d.wasSize(), 0);

  ASSERT_FALSE(d.pop_back_get(val));

  ASSERT_NO_THROW(d.push_back(1));
  ASSERT_NO_THROW(d.push_front(2));
  ASSERT_EQ(d.wasSize(), 2);
  ASSERT_EQ(d.front(), 2);
  ASSERT_EQ(d.back(), 1);
  ASSERT_NO_THROW(d.pop_back());
  ASSERT_EQ(d.front(), 2);
  ASSERT_EQ(d.back(), 2);
  ASSERT_EQ(d.wasSize(), 1);
  ASSERT_NO_THROW(d.pop_back());
  ASSERT_EQ(d.wasSize(), 0);

  ASSERT_NO_THROW(d.push_back(1));
  ASSERT_NO_THROW(d.push_front(2));
  ASSERT_EQ(d.wasSize(), 2);
  ASSERT_EQ(d.front(), 2);
  ASSERT_EQ(d.back(), 1);
  ASSERT_NO_THROW(d.pop_front());
  ASSERT_EQ(d.front(), 1);
  ASSERT_EQ(d.back(), 1);
  ASSERT_EQ(d.wasSize(), 1);
  ASSERT_NO_THROW(d.pop_front());
  ASSERT_EQ(d.wasSize(), 0);
}

TEST(concurrent, dequeConcurrency)
{
 Deque<size_t> d;

 size_t elementCount = 4096;
 size_t expectedSum = elementCount * ( elementCount - 1);
 std::atomic<size_t> actualSum = 0;

 #pragma omp parallel for
 for (size_t i = 0; i < elementCount; i++)
 {
  d.push_front(i);
  d.push_back(i);
 }

 ASSERT_EQ(d.wasSize(), elementCount * 2);

 #pragma omp parallel for
 for (size_t i = 0; i < elementCount; i++)
 {
  size_t value = 0;
  d.pop_front_get(value);
  actualSum += value;
  d.pop_back_get(value);
  actualSum += value;
 }

 ASSERT_EQ(d.wasSize(), 0);
 ASSERT_EQ(actualSum, expectedSum);
}
TEST(drainBuffer, frontAndBackClaims)
{
  jaffarCommon::concurrent::DrainBuffer<int> d;
  d.reserve(10);
  for (int i = 0; i < 10; i++) d.push_back_no_lock(i);
  ASSERT_EQ(d.wasSize(), 10);

  // A batch from the front comes in order; a batch from the back comes last element first
  int front[3];
  ASSERT_EQ(d.pop_front_get_batch(front, 3), 3);
  ASSERT_EQ(front[0], 0);
  ASSERT_EQ(front[2], 2);
  int back[4];
  ASSERT_EQ(d.pop_back_get_batch(back, 4), 4);
  ASSERT_EQ(back[0], 9);
  ASSERT_EQ(back[3], 6);
  ASSERT_EQ(d.wasSize(), 3);

  // The ends never overlap: only what is left is handed out
  int rest[8];
  ASSERT_EQ(d.pop_back_get_batch(rest, 8), 3);
  ASSERT_EQ(rest[0], 5);
  ASSERT_EQ(rest[2], 3);
  int single;
  ASSERT_FALSE(d.pop_back_get(single));
  ASSERT_FALSE(d.pop_front_get(single));

  // Refill after clear()
  d.clear();
  d.push_back_no_lock(42);
  ASSERT_TRUE(d.pop_back_get(single));
  ASSERT_EQ(single, 42);
}

TEST(drainBuffer, concurrentDrainHandsOutEachElementOnce)
{
  const int                                  n = 200000;
  jaffarCommon::concurrent::DrainBuffer<int> d;
  d.reserve(n);
  for (int i = 0; i < n; i++) d.push_back_no_lock(i);

  std::vector<std::atomic<int>> seen(n);
  for (auto& s : seen) s = 0;
#pragma omp parallel num_threads(8)
  {
    int  buffer[16];
    bool fromBack = omp_get_thread_num() % 2 == 1;
    while (true)
    {
      const size_t count = fromBack ? d.pop_back_get_batch(buffer, 7) : d.pop_front_get_batch(buffer, 16);
      if (count == 0) break;
      for (size_t i = 0; i < count; i++) seen[buffer[i]]++;
    }
  }
  for (int i = 0; i < n; i++) ASSERT_EQ(seen[i].load(), 1) << "element " << i;
}
