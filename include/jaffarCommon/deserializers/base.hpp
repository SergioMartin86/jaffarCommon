#pragma once

/**
 * @file base.hpp
 * @brief Contains the base class for the data deserializers
 */

#include <cstddef>
#include <cstdint>

namespace jaffarCommon
{

namespace deserializer
{

/**
 * Base class for deserializers
 *
 * A deserializer receives a read-only input data buffer upon creation, and unfolds it onto an actual object
 * element by element, by offering different types of 'pop' operations.
 */
class Base
{
  public:

  /**
   * Default constructor for the deserializer classes
   *
   * @param[in] inputDataBuffer The input buffer from whence to read the input data
   * @param[in] inputDataBufferSize The size of the input buffer
   */
  Base(const void *__restrict inputDataBuffer, const size_t inputDataBufferSize)
    : _inputDataBuffer((const uint8_t *)inputDataBuffer)
    , _inputDataBufferSize(inputDataBufferSize)
  {}

  virtual ~Base() = default;

  /**
   * Deserializes the specified number of bytes onto the output data buffer, popping the information from the input buffer
   *
   * @note The input buffer may be compressed or non-contiguous
   *
   * @param[out] outputDataBuffer The contiguous output buffer onto which to deserialize.
   * @param[in] outputDataBufferSize The number of bytes to save onto the output buffer
   */
  virtual void pop(void *const __restrict outputDataBuffer, const size_t outputDataBufferSize) = 0;

  /**
   * Deserializes the specified number of contiguous bytes onto the output data buffer, popping the information from the input buffer
   *
   * @note This function forces the treatment of the input and output as uncompressed contiguous buffers
   *
   * @param[out] outputDataBuffer The contiguous output buffer onto which to deserialize
   * @param[in] outputDataBufferSize The number of bytes to save onto the output buffer
   */
  virtual void popContiguous(void *const __restrict outputDataBuffer, const size_t outputDataBufferSize) = 0;

  /**
   * Get the position of the input buffer header.
   *
   * @return The position of the input buffer header. This value represents the size of the input data at the end of the deserialization process
   */
  __INLINE__ size_t getInputSize() const { return _inputDataBufferPos; }

  /**
   * Gets a reference to the input data buffer
   *
   * @return The pointer to the input data buffer
   */
  __INLINE__ const uint8_t *getInputDataBuffer() const { return _inputDataBuffer; }

  protected:

  /**
   *  The read-only internally-stored input data buffer
   */
  const uint8_t *__restrict const _inputDataBuffer;

  /**
   *  The maximum size of the input data buffer
   */
  const size_t _inputDataBufferSize;

  /**
   * The position of the header that iterates over the data buffer
   */
  size_t _inputDataBufferPos = 0;
};

} // namespace deserializer

} // namespace jaffarCommon