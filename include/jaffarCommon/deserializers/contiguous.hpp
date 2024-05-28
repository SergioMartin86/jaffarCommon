#pragma once

/**
 * @file contiguous.hpp
 * @brief Contains the contiguous data deserializer
 */

#include <string.h>
#include <limits>
#include "../exceptions.hpp"
#include "base.hpp"

namespace jaffarCommon
{

namespace deserializer
{

/**
 * The contiguous deserialization class assumes that both the input buffer and output elements are contiguous and
 * haven't been through any compression.
 *
 *
 */
class Contiguous final : public deserializer::Base
{
  public:

  /**
   * Default constructor for the contiguous deserializer class
   *
   * @param[in] inputDataBuffer The input buffer from whence to read the input data
   * @param[in] inputDataBufferSize The size of the input buffer
   */
  Contiguous(const void *__restrict inputDataBuffer = nullptr, const size_t inputDataBufferSize = std::numeric_limits<uint32_t>::max())
    : deserializer::Base(inputDataBuffer, inputDataBufferSize)
  {}

  ~Contiguous() = default;

  __INLINE__ void popContiguous(void *const __restrict outputDataBuffer, const size_t count) override
  {
    // Making sure we do not exceed the maximum size estipulated
    if (_inputDataBufferPos + count > _inputDataBufferSize)
      JAFFAR_THROW_RUNTIME(
        "Maximum input data position reached (%lu) by current position (%lu) + count (%lu) before contiguous deserialization", _inputDataBufferSize, _inputDataBufferPos, count);

    // Only perform memcpy if the input block is not null
    if (outputDataBuffer != nullptr && _inputDataBuffer != nullptr) memcpy(outputDataBuffer, &_inputDataBuffer[_inputDataBufferPos], count);

    // Moving input data pointer position
    _inputDataBufferPos += count;
  }

  __INLINE__ void pop(void *const __restrict outputDataBuffer, const size_t count) override { popContiguous(outputDataBuffer, count); }
};

} // namespace deserializer

} // namespace jaffarCommon