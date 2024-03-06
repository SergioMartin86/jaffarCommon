#pragma once

/**
 * @file differential.hpp
 * @brief Contains the differential data deserializer
 */

#include "../exceptions.hpp"
#include "base.hpp"
#include <xdelta3/xdelta3.h>

namespace jaffarCommon
{

namespace deserializer
{

class Differential final : public deserializer::Base
{
  public:
  Differential(
    const void *__restrict inputDataBuffer = nullptr,
    const size_t inputDataBufferSize = std::numeric_limits<uint32_t>::max(),
    const void *__restrict referenceDataBuffer = nullptr,
    const size_t referenceDataBufferSize = std::numeric_limits<uint32_t>::max(),
    const bool useZlib = false) : deserializer::Base(inputDataBuffer, inputDataBufferSize),
                                  _referenceDataBuffer((const uint8_t *)referenceDataBuffer),
                                  _referenceDataBufferSize(referenceDataBufferSize),
                                  _useZlib(useZlib)
  {
  }

  ~Differential() = default;

  __INLINE__ void popContiguous(void *const __restrict outputDataBuffer, const size_t outputDataSize) override
  {
    // Making sure we do not exceed the maximum size estipulated
    if (_inputDataBufferPos + outputDataSize > _inputDataBufferSize) JAFFAR_THROW_RUNTIME("Maximum input data position reached before contiguous deserialization of (%lu + %lu > %lu) bytes", _inputDataBufferPos, outputDataSize, _inputDataBufferSize);
    if (_referenceDataBufferPos + outputDataSize > _referenceDataBufferSize) JAFFAR_THROW_RUNTIME("[Error] Maximum reference data position to be exceeded on contiguous deserialization (%lu + %lu > %lu)", _referenceDataBufferPos, outputDataSize, _referenceDataBufferSize);

    // Only perform memcpy if the input block is not null
    if (_inputDataBuffer != nullptr) memcpy(outputDataBuffer, &_inputDataBuffer[_inputDataBufferPos], outputDataSize);

    // Moving input data pointer position
    _inputDataBufferPos += outputDataSize;

    // Moving reference data buffer position
    _referenceDataBufferPos += outputDataSize;
  }

  __INLINE__ void pop(void *const __restrict outputDataBuffer, const size_t outputDataSize) override
  {
    if (outputDataBuffer == nullptr) return;

    // Reading differential count
    usize_t diffCount = *(usize_t *)&_inputDataBuffer[_inputDataBufferPos];

    // Size of differential buffer size
    const size_t differentialBufferSize = sizeof(usize_t);

    // If we reached maximum output, stop here
    if (_inputDataBufferPos + differentialBufferSize >= _inputDataBufferSize) JAFFAR_THROW_RUNTIME("[Error] Maximum input data position reached before differential buffer size decode (%lu + %lu > %lu)", _inputDataBufferPos, differentialBufferSize, _inputDataBufferSize);

    // Advancing position pointer to store the difference counter
    _inputDataBufferPos += differentialBufferSize;

    // If we reached maximum output, stop here
    if (_referenceDataBufferPos + outputDataSize > _referenceDataBufferSize) JAFFAR_THROW_RUNTIME("[Error] Maximum reference data position exceeded before differential decode (%lu + %lu > %lu)", _referenceDataBufferPos, outputDataSize, _referenceDataBufferSize);

    // Encoding differential
    usize_t output_size;
    int ret = xd3_decode_memory(
      &_inputDataBuffer[_inputDataBufferPos],
      diffCount,
      &_referenceDataBuffer[_referenceDataBufferPos],
      outputDataSize,
      (uint8_t *)outputDataBuffer,
      &output_size,
      outputDataSize,
      _useZlib ? 0 : XD3_NOCOMPRESS);

    // If an error happened, print it here
    if (ret != 0) JAFFAR_THROW_RUNTIME("[Error] unexpected error while decoding differential decompression. Probably maximum input data position reached after differential decode (%lu + %lu > %lu)", _inputDataBufferPos, diffCount, _inputDataBufferSize);

    // Increasing output data position pointer
    _inputDataBufferPos += diffCount;

    // Finally, increasing reference data position pointer
    _referenceDataBufferPos += outputDataSize;
  }

  size_t getReferenceDataBufferPos() const { return _referenceDataBufferPos; }

  private:
  const uint8_t *__restrict const _referenceDataBuffer;
  const size_t _referenceDataBufferSize;
  size_t _referenceDataBufferPos = 0;
  const bool _useZlib;
};

} // namespace deserializer

} // namespace jaffarCommon