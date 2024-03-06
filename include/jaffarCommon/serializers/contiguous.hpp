#pragma once

/**
 * @file contiguous.hpp
 * @brief Contains the contiguous data serializer
 */

#include "../exceptions.hpp"
#include "base.hpp"
#include <cstring>
#include <limits>

namespace jaffarCommon
{

namespace serializer
{

class Contiguous final : public serializer::Base
{
  public:
  Contiguous(
    void *__restrict outputDataBuffer = nullptr,
    const size_t outputDataBufferSize = std::numeric_limits<uint32_t>::max()) : serializer::Base(outputDataBuffer, outputDataBufferSize)
  {
  }

  ~Contiguous() = default;

  __INLINE__ void pushContiguous(const void *const __restrict inputData, const size_t inputDataSize) override
  {
    // Making sure we do not exceed the maximum size estipulated
    if (_outputDataBufferPos + inputDataSize > _outputDataBufferSize) JAFFAR_THROW_RUNTIME("Maximum output data position (%lu) reached before contiguous serialization from pos (%lu) and input size (%lu)", _outputDataBufferSize, _outputDataBufferPos, inputDataSize);

    // Only perform memcpy if the output block is not null
    if (_outputDataBuffer != nullptr) memcpy(&_outputDataBuffer[_outputDataBufferPos], inputData, inputDataSize);

    // Moving output data pointer position
    _outputDataBufferPos += inputDataSize;
  }

  __INLINE__ void push(const void *const __restrict inputData, const size_t inputDataSize) override
  {
    pushContiguous(inputData, inputDataSize);
  }
};

} // namespace serializer

} // namespace jaffarCommon