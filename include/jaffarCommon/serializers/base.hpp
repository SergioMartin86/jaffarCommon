#pragma once

/**
 * @file base.hpp
 * @brief Contains the base class for the data serializers
 */

#include <stddef.h>
#include <stdint.h>

namespace jaffarCommon
{

namespace serializer
{

/**
 * Base class for serializers
 *
 * A serializer receives a output data buffer upon creation, and allows the user to fill it up with different elements (e.g. attributes of a class)
 * by repeated calls to the different 'push' functions.
 */
class Base
{
public:
  /**
   * Default constructor for the serializer classes
   *
   * @param[in] outputDataBuffer The input buffer from whence to read the input data
   * @param[in] outputDataBufferSize The size of the input buffer
   */
  Base(void* __restrict outputDataBuffer, const size_t outputDataBufferSize) : _outputDataBuffer((uint8_t*)outputDataBuffer), _outputDataBufferSize(outputDataBufferSize) {}

  virtual ~Base() = default;

  /**
   * Serializes the specified number of bytes onto the output data buffer, pushing the information from the input data buffer
   *
   * @note The input buffer may be compressed or non-contiguous
   *
   * @param[out] inputDataBuffer The contiguous output buffer from which data is serialized. Passing nullptr is allowed and can be used to determine the required output buffer size
   * @param[in] inputDataSize The number of bytes from the input data buffer to serialize
   */
  virtual void push(const void* const __restrict inputDataBuffer = nullptr, const size_t inputDataSize = 0) = 0;

  /**
   * Serializes the specified number of contiguous bytes onto the output data buffer, pushing the information from the input buffer
   *
   * @note This function forces the treatment of the input and output as uncompressed contiguous buffers
   *
   * @param[out] inputDataBuffer The contiguous output buffer from which data is serialized. Passing nullptr is allowed and can be used to determine the required output buffer size
   * @param[in] inputDataSize The number of bytes from the input data buffer to serialize
   */
  virtual void pushContiguous(const void* const __restrict inputDataBuffer = nullptr, const size_t inputDataSize = 0) = 0;

  /**
   *  The internally-stored output data buffer size
   *
   * @return The size of the output data so far (at the end, this represents the output buffer size)
   */
  __JAFFAR_COMMON_INLINE__ size_t getOutputSize() const { return _outputDataBufferPos; }

  /**
   *  The internally-stored output data buffer size
   *
   * @return A reference to the output data buffer
   */
  __JAFFAR_COMMON_INLINE__ uint8_t* getOutputDataBuffer() const { return _outputDataBuffer; }

protected:
  /**
   *  The write-only internally stored output data buffer
   */
  uint8_t* __restrict const _outputDataBuffer;

  /**
   * The size of the output data buffer
   */
  const size_t _outputDataBufferSize;

  /**
   * The current header position of the output data buffer (how much was used)
   */
  size_t _outputDataBufferPos = 0;
};

} // namespace serializer

} // namespace jaffarCommon