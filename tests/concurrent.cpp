#include <atomic>
#include "gtest/gtest.h"
#include <jaffarCommon/concurrent.hpp>

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