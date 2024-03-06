#pragma once

/**
 * @file base.hpp
 * @brief Contains the base class for the data serializers
 */

#include <cstddef>
#include <cstdint>

namespace jaffarCommon
{

namespace serializer
{

class Base
{
  public:
  Base(
    void *__restrict outputDataBuffer,
    const size_t outputDataBufferSize) : _outputDataBuffer((uint8_t *)outputDataBuffer),
                                         _outputDataBufferSize(outputDataBufferSize)
  {
  }

  virtual ~Base() = default;

  virtual void push(const void *const __restrict inputData, const size_t inputDataSize) = 0;
  virtual void pushContiguous(const void *const __restrict inputData, const size_t inputDataSize) = 0;
  __INLINE__ size_t getOutputSize() const { return _outputDataBufferPos; }
  __INLINE__ uint8_t *getOutputDataBuffer() const { return _outputDataBuffer; }

  protected:
  uint8_t *__restrict const _outputDataBuffer;
  const size_t _outputDataBufferSize;
  size_t _outputDataBufferPos = 0;
};

} // namespace serializer

} // namespace jaffarCommon