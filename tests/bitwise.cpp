#include "../include/bitwise.hpp"
#include "gtest/gtest.h"

using namespace jaffarCommon::bitwise;

TEST(bitwise, setBitValueIncrement)
{
  // Testing exponential increments in a 64-bit value
  uint64_t testValue    = 0;
  uint64_t controlValue = 0;
  uint64_t increment    = 1;

  ASSERT_EQ(testValue, controlValue);
  for (size_t i = 0; i < 64; i++)
  {
    // Setting bit
    setBitValue(&testValue, i, true);
    
    // Advancing control value
    controlValue += increment;
    
    // Checking values are still equal
    ASSERT_EQ(testValue, controlValue);

    // Exponentiating increment
    increment <<= 1;
  }
}

TEST(bitwise, setBitValueDecrement)
{
  // Testing exponential decrements in a 64-bit value
  uint64_t testValue    = 0b1111111111111111111111111111111111111111111111111111111111111111;
  uint64_t controlValue = 0b1111111111111111111111111111111111111111111111111111111111111111;
  uint64_t decrement    = 0b1000000000000000000000000000000000000000000000000000000000000000;

  ASSERT_EQ(testValue, controlValue);
  for (ssize_t i = 63; i >= 0; i--)
  {
    // Setting bit
    setBitValue(&testValue, i, false);
    
    // Advancing control value
    controlValue -= decrement;
    
    // Checking values are still equal
    ASSERT_EQ(testValue, controlValue);

    // Halving decrement
    decrement >>= 1;
  }
}

TEST(bitwise, getBitValueIncrement)
{
  // Testing exponential decrements in a 64-bit value
  uint64_t value     = 0;
  uint64_t increment = 1;

  for (ssize_t i = 0; i < 64; i++)
  {
    // Checking the corresponding bit is up
    ASSERT_FALSE(getBitValue(&value, i));

    // Advancing control value
    value += increment;
    
    // Checking the corresponding bit is down
    ASSERT_TRUE(getBitValue(&value, i));

    // Exponentiating decrement
    increment <<= 1;
  }
}

TEST(bitwise, getBitValueDecrement)
{
  // Testing exponential decrements in a 64-bit value
  uint64_t value     = 0b1111111111111111111111111111111111111111111111111111111111111111;
  uint64_t decrement = 0b1000000000000000000000000000000000000000000000000000000000000000;

  for (ssize_t i = 63; i >= 0; i--)
  {
    // Checking the corresponding bit is up
    ASSERT_TRUE(getBitValue(&value, i));

    // Advancing control value
    value -= decrement;
    
    // Checking the corresponding bit is down
    ASSERT_FALSE(getBitValue(&value, i));

    // Halving decrement
    decrement >>= 1;
  }
}