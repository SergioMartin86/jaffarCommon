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

/**
 * A reference table that maps a given bit index to its corresponding bit map (one in the indicates position, zeros in all other bits)
*/
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

/**
 * A reference table that maps a given bit index to its corresponding negated bit map (zero in the indicates position, ones in all other bits)
*/
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

/**
 * This function copies a contiguous set of k elements of size n bits from a source buffer plus an offset s_off * n to a destination buffer plus an offset of d_off * n
 * 
 * @param[out] dstBufferPtr The destination buffer
 * @param[in] dstBufferSize The size of the destination buffer (expressed in bytes). This is necessary to guard possible overflows before they happen
 * @param[in] dstBufferOffset The size of the destination buffer (expressed in elements)
 * @param[in] srcBufferPtr The source buffer
 * @param[in] srcBufferSize The size of the source buffer (expressed in bytes). This is necessary to guard possible overflows before they happen
 * @param[in] srcBufferOffset The size of the source buffer (expressed in elements)
 * @param[in] count The number of elements to copy
 * @param[in] elementBitSize The size of each element (expressed in bits)
*/
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

/**
 * Calculates the number of bits necessary encode a number of distinct elements.
 * 
 * @param[in] elementCount The number of elements to encode
 * @return The number of bits required. Examples: (Element Count, Bits Required) => (0,0), (1,0), (2,1), (3,2), (4,2), (5,3), (6,3), (7,3), (8,3), (9,4)
 */
__INLINE__ size_t getEncodingBitsForElementCount(const size_t elementCount)
{
  // Calculating bit storage for the possible inputs index
  size_t bitEncodingSize = 0;
  size_t encodingCapacity = 1;
  while (encodingCapacity < elementCount) { encodingCapacity <<= 1, bitEncodingSize++; };
  return bitEncodingSize;
}

/**
 * Calculates how many bytes if needed to store a certain amount of bits
 * 
 * @param[in] bitCount The number of bits to store
 * @return The bytes required to store them
*/
__INLINE__ size_t getByteStorageForBitCount(const size_t bitCount)
{
  // Calculating bit storage for the possible inputs index
  size_t byteStorageSize = bitCount / 8;
  if (bitCount % 8 > 0) byteStorageSize++;
  return byteStorageSize;
}

/**
 * Sets to value for a given bit inside a buffer of any size
 * 
 * @param[out] dst The buffer containing the bit to set
 * @param[in] idx Index (position) of the bit to set inside the buffer
 * @param[in] value Value to set (true or false)
*/
__INLINE__ void setBitValue(void *dst, const size_t idx, const bool value)
{
  size_t dstPosByte = idx / 8;
  uint8_t dstPosBit = idx % 8;
  auto dstPtr = (uint8_t *)dst;

  if (value == false) dstPtr[dstPosByte] = dstPtr[dstPosByte] & bitNotMaskTable[dstPosBit];
  if (value == true) dstPtr[dstPosByte] = dstPtr[dstPosByte] | bitMaskTable[dstPosBit];
}

/**
 * Gets the value of a given bit inside a buffer of any size
 * 
 * @param[in] src The buffer containing the bit to set
 * @param[in] idx Index (position) of the bit to get inside value
 * @return Whether the specified bit was true or false
 */
__INLINE__ bool getBitValue(const void *src, const size_t idx)
{
  size_t srcPosByte = idx / 8;
  uint8_t srcPosBit = idx % 8;
  auto srcPtr = (const uint8_t *)src;

  return (srcPtr[srcPosByte] & bitMaskTable[srcPosBit]) > 0;
}

/**
 * Gets the value of a given bit inside an 8-bit word
 * 
 * @param[in] value The 8-bit word
 * @param[in] idx Index (position) of the bit to get inside the value (only accepted values: 0-7)
 * @return Whether the specified bit was true or false
 */
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