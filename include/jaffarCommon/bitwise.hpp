#pragma once

/**
 * @file bitwise.hpp
 * @brief Contains common functions related to bitwise operations
 */

#include "exceptions.hpp"
#include <cstdint>
#include <stddef.h>

namespace jaffarCommon
{

namespace bitwise
{

uint8_t bitMaskTable[8] =
  {
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000};

uint8_t bitNotMaskTable[8] =
  {
    0b11111110,
    0b11111101,
    0b11111011,
    0b11110111,
    0b11101111,
    0b11011111,
    0b10111111,
    0b01111111};

__INLINE__ void bitcopy(void *dstBufferPtr, const size_t dstBufferSize, const size_t dstBufferOffset, const void *srcBufferPtr, const size_t srcBufferSize, const size_t srcBufferOffset, const size_t count, const size_t elementBitSize)
{
  if (elementBitSize == 0) JAFFAR_THROW_LOGIC("Element bit size must be a positive number greater than zero");
  if (dstBufferOffset + elementBitSize * count > dstBufferSize) JAFFAR_THROW_LOGIC("The operation will overflow destination buffer (%lu + %lu * %lu > %lu)", dstBufferOffset, elementBitSize, count, dstBufferSize);
  if (srcBufferOffset + elementBitSize * count > srcBufferSize) JAFFAR_THROW_LOGIC("The operation will overflow source buffer (%lu + %lu * %lu > %lu)", srcBufferOffset, elementBitSize, count, srcBufferSize);

  uint8_t *dstBuffer = (uint8_t *)dstBufferPtr;
  const uint8_t *srcBuffer = (const uint8_t *)srcBufferPtr;
  const size_t totalBitCount = count * elementBitSize;
  const size_t dstOffsetBits = dstBufferOffset * elementBitSize;
  const size_t srcOffsetBits = srcBufferOffset * elementBitSize;
  size_t dstPosByte = dstOffsetBits / 8;
  uint8_t dstPosBit = dstOffsetBits % 8;
  size_t srcPosByte = srcOffsetBits / 8;
  uint8_t srcPosBit = srcOffsetBits % 8;

  for (size_t i = 0; i < totalBitCount; i++)
  {
    // Clear bit in question
    dstBuffer[dstPosByte] = dstBuffer[dstPosByte] & bitNotMaskTable[dstPosBit];

    // If the corresponding bit is set in source, set it up in dst
    if ((srcBuffer[srcPosByte] & bitMaskTable[srcPosBit]) > 0) dstBuffer[dstPosByte] = dstBuffer[dstPosByte] | bitMaskTable[dstPosBit];

    // Advance bit positions
    dstPosBit++;
    srcPosBit++;

    // If crossed a byte barrier, go over the next byte
    if (dstPosBit == 8)
    {
      dstPosBit = 0;
      dstPosByte++;
    }
    if (srcPosBit == 8)
    {
      srcPosBit = 0;
      srcPosByte++;
    }
  }
}

__INLINE__ size_t getEncodingBitsForElementCount(const size_t elementCount)
{
  // Calculating bit storage for the possible inputs index
  size_t bitEncodingSize = 0;
  size_t encodingCapacity = 1;
  while (encodingCapacity < elementCount) { encodingCapacity <<= 1, bitEncodingSize++; };
  return bitEncodingSize;
}

__INLINE__ size_t getByteStorageForBitCount(const size_t bitCount)
{
  // Calculating bit storage for the possible inputs index
  size_t byteStorageSize = bitCount / 8;
  if (bitCount % 8 > 0) byteStorageSize++;
  return byteStorageSize;
}

__INLINE__ void setBitValue(void *dst, const size_t idx, const bool value)
{
  size_t dstPosByte = idx / 8;
  uint8_t dstPosBit = idx % 8;
  auto dstPtr = (uint8_t *)dst;

  if (value == false) dstPtr[dstPosByte] = dstPtr[dstPosByte] & bitNotMaskTable[dstPosBit];
  if (value == true) dstPtr[dstPosByte] = dstPtr[dstPosByte] | bitMaskTable[dstPosBit];
}

__INLINE__ bool getBitValue(const void *src, const size_t idx)
{
  size_t srcPosByte = idx / 8;
  uint8_t srcPosBit = idx % 8;
  auto srcPtr = (const uint8_t *)src;

  return (srcPtr[srcPosByte] & bitMaskTable[srcPosBit]) > 0;
}

__INLINE__ bool getBitFlag(const uint8_t value, const uint8_t idx)
{
  if (idx > 7) JAFFAR_THROW_LOGIC("Provided bit index higher than 7 for a an 8-bit value");

  if (((idx == 7) && (value & 0b10000000)) ||
      ((idx == 6) && (value & 0b01000000)) ||
      ((idx == 5) && (value & 0b00100000)) ||
      ((idx == 4) && (value & 0b00010000)) ||
      ((idx == 3) && (value & 0b00001000)) ||
      ((idx == 2) && (value & 0b00000100)) ||
      ((idx == 1) && (value & 0b00000010)) ||
      ((idx == 0) && (value & 0b00000001))) return true;
  return false;
}

} // namespace bitwise

} // namespace jaffarCommon