#pragma once

/**
 * @file contiguous.hpp
 * @brief Contains the contiguous data serializer
 */

#include <cstring>
#include <limits>
#include "../exceptions.hpp"
#include "base.hpp"

namespace jaffarCommon
{

namespace serializer
{

/**
 * The contiguous serialization class assumes that both the input elements and output buffers are contiguous and
 * haven't been through any compression.
 */
class Contiguous final : public serializer::Base
{
  public:

  /**
   * Default constructor for the contiguous serializer class
   *
   * @param[in] outputDataBuffer The output buffer onto which to write the output data
   * @param[in] outputDataBufferSize The size of the output buffer (not to be exceeded)
   */
  Contiguous(void *__restrict outputDataBuffer = nullptr, const size_t outputDataBufferSize = std::numeric_limits<uint32_t>::max())
    : serializer::Base(outputDataBuffer, outputDataBufferSize)
  {}

  ~Contiguous() = default;

  __INLINE__ void pushContiguous(const void *const __restrict inputDataBuffer = nullptr, const size_t inputDataSize = 0) override
  {
    // Making sure we do not exceed the maximum size estipulated
    if (_outputDataBufferPos + inputDataSize > _outputDataBufferSize)
      JAFFAR_THROW_RUNTIME("Maximum output data position (%lu) reached before contiguous serialization from pos (%lu) and input size (%lu)",
                           _outputDataBufferSize,
                           _outputDataBufferPos,
                           inputDataSize);

    // Only perform memcpy if the output block is not null
    if (_outputDataBuffer != nullptr && inputDataBuffer != nullptr) memcpy(&_outputDataBuffer[_outputDataBufferPos], inputDataBuffer, inputDataSize);

    // Moving output data pointer position
    _outputDataBufferPos += inputDataSize;
  }

  __INLINE__ void push(const void *const __restrict inputDataBuffer, const size_t inputDataSize) override { pushContiguous(inputDataBuffer, inputDataSize); }
};

} // namespace serializer

} // namespace jaffarCommon