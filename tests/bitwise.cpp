#include <random>
#include <cstdint>
#include "gtest/gtest.h"
#include "../include/bitwise.hpp"

using namespace jaffarCommon::bitwise;

TEST(bitwise, bitcopyNoByteCrossing)
{
  uint8_t dstBuffer;
  uint8_t srcBuffer;
  const uint8_t dstBufferSize = 8;
  const uint8_t srcBufferSize = 8;
  uint64_t elementBitSize;
  uint64_t dstBufferOffset;
  uint64_t srcBufferOffset;
  uint64_t count;

  // Filling gap
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 1;
  dstBufferOffset = 3;
  srcBufferOffset = 3;
  count = 2;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b11111111);

  // With element bit size == 2
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 2;
  dstBufferOffset = 1;
  srcBufferOffset = 1;
  count = 2;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b11011011);

  // Full replacement (element bit size 1)
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 1;
  dstBufferOffset = 0;
  srcBufferOffset = 0;
  count = 8;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, srcBuffer);

  // Full replacement (element bit size 2)
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 2;
  dstBufferOffset = 0;
  srcBufferOffset = 0;
  count = 4;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, srcBuffer);

  // Full replacement (element bit size 4)
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 4;
  dstBufferOffset = 0;
  srcBufferOffset = 0;
  count = 2;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, srcBuffer);

  // Full replacement (element bit size 8)
  dstBuffer = 0b11100111;
  srcBuffer = 0b00000000;
  elementBitSize = 8;
  dstBufferOffset = 0;
  srcBufferOffset = 0;
  count = 1;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b00000000);

  // First half replacement
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 4;
  dstBufferOffset = 0;
  srcBufferOffset = 0;
  count = 1;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b11101000);

  // Second half replacement
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 4;
  dstBufferOffset = 1;
  srcBufferOffset = 1;
  count = 1;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b00010111);

  // First half replacement (cross pattern)
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 4;
  dstBufferOffset = 1;
  srcBufferOffset = 0;
  count = 1;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b10000111);

  // Second half replacement (cross pattern)
  dstBuffer = 0b11100111;
  srcBuffer = 0b00011000;
  elementBitSize = 4;
  dstBufferOffset = 0;
  srcBufferOffset = 1;
  count = 1;
  ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b11100001);
}

TEST(bitwise, bitcopyByteCrossing)
{
  uint64_t dstBuffer = 0b0000000000000000000000000000000000000000000000000000000000000000;
  uint8_t srcBuffer = 0b00111000;
  const uint8_t dstBufferSize = 64;
  const uint8_t srcBufferSize = 8;
  uint64_t elementBitSize = 3;
  uint64_t srcBufferOffset = 1;
  uint64_t count = 1;

  for (size_t i = 2; i * elementBitSize < 64; i+= 3) ASSERT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, i, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));
  ASSERT_EQ(dstBuffer, 0b0111000000111000000111000000111000000111000000111000000111000000);
}

TEST(bitwise, bitcopyRandom)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  auto sizeRNG = std::uniform_int_distribution<uint64_t>(1, 32768);

  // Doing valid random copies of big arrays
  for (size_t i = 0; i < 1024; i++)
  {
    uint64_t dstBufferSize = sizeRNG(gen);
    uint64_t srcBufferSize = sizeRNG(gen);
    auto dstBuffer = malloc(dstBufferSize);
    auto srcBuffer = malloc(srcBufferSize);

    uint64_t elementBitSize = sizeRNG(gen) % std::min(srcBufferSize, dstBufferSize);
    if (elementBitSize == 0) elementBitSize = 1;

    size_t maxDstElements = dstBufferSize / elementBitSize;
    size_t maxSrcElements = srcBufferSize / elementBitSize;

    uint64_t dstBufferOffset = sizeRNG(gen) % maxDstElements;
    uint64_t srcBufferOffset = sizeRNG(gen) % maxSrcElements;

    size_t maxDstCount = maxDstElements - dstBufferOffset;
    size_t maxSrcCount = maxSrcElements - srcBufferOffset;

    uint64_t count = sizeRNG(gen) % std::min(maxDstCount, maxSrcCount);

    ASSERT_NO_THROW(bitcopy(dstBuffer, dstBufferSize, dstBufferOffset, srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));

    free(dstBuffer);
    free(srcBuffer);
  }
}

TEST(bitwise, bitcopyBadInpus)
{
  uint64_t dstBuffer;
  uint64_t srcBuffer;
  uint64_t dstBufferSize;
  uint64_t srcBufferSize;
  uint64_t elementBitSize;
  uint64_t dstBufferOffset;
  uint64_t srcBufferOffset;
  uint64_t count;

  auto resetGoodValues = [&]()
  {
    dstBuffer = 0;
    srcBuffer = 0;
    dstBufferSize = 64;
    srcBufferSize = 64;
    elementBitSize = 1;
    dstBufferOffset = 0;
    srcBufferOffset = 0;
    count = 32;
  };

  // Control: Good values
  resetGoodValues();
  EXPECT_NO_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize));

  // Error: elementBitSize is zero
  resetGoodValues();
  elementBitSize = 0;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);

  // Error: Dst buffer exceeded by size
  resetGoodValues();
  dstBufferSize = 31;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);

  // Error: Src buffer exceeded by size
  resetGoodValues();
  srcBufferSize = 31;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);

  // Error: Dst buffer exceeded by offset
  resetGoodValues();
  dstBufferOffset = 33;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);

  // Error: Src buffer exceeded by offset
  resetGoodValues();
  srcBufferOffset = 33;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);

  // Error: Dst buffer exceeded by element size
  resetGoodValues();
  srcBufferSize = 1024;
  elementBitSize = 16;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);

  // Error: Src buffer exceeded by element size
  resetGoodValues();
  dstBufferSize = 1024;
  elementBitSize = 16;
  EXPECT_THROW(bitcopy(&dstBuffer, dstBufferSize, dstBufferOffset, &srcBuffer, srcBufferSize, srcBufferOffset, count, elementBitSize), std::logic_error);
}

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

TEST(bitwise, getEncodingBitsForElementCount)
{
  size_t expectedBitCount = 0;
  size_t maxElements = 1;
  for (size_t elementCount = 0; elementCount < 1024; elementCount++)
  {
    if (elementCount > maxElements) { expectedBitCount++; maxElements <<= 1; }
    ASSERT_EQ(getEncodingBitsForElementCount(elementCount), expectedBitCount);
  }
}

TEST(bitwise, getByteStorageForBitCount)
{
  for (size_t byteId = 0; byteId < 1024; byteId++)
  {
    ASSERT_EQ(getByteStorageForBitCount(byteId * 8), byteId);
    for (size_t bitId = 1; bitId < 8; bitId++)
     ASSERT_EQ(getByteStorageForBitCount(byteId * 8 + bitId), byteId + 1);
  }
}

TEST(bitwise, getBitFlag)
{
  ASSERT_FALSE(getBitFlag(0b11111110, 0));
  ASSERT_FALSE(getBitFlag(0b11111101, 1));
  ASSERT_FALSE(getBitFlag(0b11111011, 2));
  ASSERT_FALSE(getBitFlag(0b11110111, 3));
  ASSERT_FALSE(getBitFlag(0b11101111, 4));
  ASSERT_FALSE(getBitFlag(0b11011111, 5));
  ASSERT_FALSE(getBitFlag(0b10111111, 6));
  ASSERT_FALSE(getBitFlag(0b01111111, 7));

  ASSERT_TRUE(getBitFlag(0b00000001, 0));
  ASSERT_TRUE(getBitFlag(0b00000010, 1));
  ASSERT_TRUE(getBitFlag(0b00000100, 2));
  ASSERT_TRUE(getBitFlag(0b00001000, 3));
  ASSERT_TRUE(getBitFlag(0b00010000, 4));
  ASSERT_TRUE(getBitFlag(0b00100000, 5));
  ASSERT_TRUE(getBitFlag(0b01000000, 6));
  ASSERT_TRUE(getBitFlag(0b10000000, 7));

  ASSERT_THROW(getBitFlag(0, 8), std::logic_error);
}