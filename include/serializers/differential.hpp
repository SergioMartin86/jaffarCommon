#pragma once

/**
 * @file differential.hpp
 * @brief Contains the differential data serializer
 */

#include "../../extern/xdelta3/xdelta3.h"
#include <cstring>
#include <limits>
#include "base.hpp"

namespace jaffarCommon
{

namespace serializer
{

class Differential final : public serializer::Base
{
  public:

  Differential(
    void* __restrict outputDataBuffer = nullptr, 
    const size_t outputDataBufferSize = std::numeric_limits<uint32_t>::max(),
    const void* __restrict referenceDataBuffer = nullptr,
    const size_t referenceDataBufferSize = std::numeric_limits<uint32_t>::max(),
    const bool useZlib = false
  ) : serializer::Base(outputDataBuffer, outputDataBufferSize),
   _referenceDataBuffer((const uint8_t*)referenceDataBuffer),
   _referenceDataBufferSize(referenceDataBufferSize),
   _useZlib(useZlib)
  {
  }

  ~Differential() = default;

  __INLINE__ void pushContiguous(const void* const __restrict inputData, const size_t inputDataSize) override
  {
    // Only perform memcpy if the output block is not null
    if (_outputDataBuffer != nullptr) memcpy(&_outputDataBuffer[_outputDataBufferPos], inputData, inputDataSize);

    // Making sure we do not exceed the maximum size estipulated
    if (_outputDataBufferPos + inputDataSize > _outputDataBufferSize) JAFFAR_THROW_RUNTIME("Maximum output data position reached before contiguous serialization (%lu + %lu > %lu)", _outputDataBufferPos, inputDataSize, _outputDataBufferSize);
    if (_referenceDataBufferPos + inputDataSize > _referenceDataBufferSize) JAFFAR_THROW_RUNTIME("[Error] Maximum reference data position exceeded on contiguous deserialization (%lu + %lu > %lu)", _referenceDataBufferPos, inputDataSize, _referenceDataBufferSize);

    // Moving output data pointer position
    _outputDataBufferPos += inputDataSize;

    // Moving reference data pointer position
    _referenceDataBufferPos += inputDataSize;
  }

  __INLINE__ void push(const void* const __restrict inputData, const size_t inputDataSize) override
  {
    // If output data buffer is null, then we simply ignore differential data.
    if (_outputDataBuffer == nullptr) return;

    // Check that we don't exceed reference data size
    if (_referenceDataBufferPos + inputDataSize > _referenceDataBufferSize) JAFFAR_THROW_RUNTIME("[Error] Differential compression size exceeds reference data buffer size (%lu + %lu > %lu)", _referenceDataBufferPos, inputDataSize, _referenceDataBufferSize);

    // Variable to store difference count 
    auto diffCount = (usize_t*)&_outputDataBuffer[_outputDataBufferPos];

    // Size of differential buffer size
    const size_t differentialBufferSize = sizeof(usize_t);

    // If we reached maximum output, stop here
    if (_outputDataBufferPos + differentialBufferSize >= _outputDataBufferSize) JAFFAR_THROW_RUNTIME("[Error] Maximum output data position reached before differential encode  (%lu + %lu > %lu)", _outputDataBufferPos, differentialBufferSize, _outputDataBufferSize);

    // Advancing position pointer to store the difference counter
    _outputDataBufferPos += differentialBufferSize;

    // Encoding differential
    int ret = xd3_encode_memory(
      (const uint8_t*)inputData,
      inputDataSize,
      &_referenceDataBuffer[_referenceDataBufferPos],
      inputDataSize,
      &_outputDataBuffer[_outputDataBufferPos],
      diffCount,
      _outputDataBufferSize - _outputDataBufferPos,
      _useZlib ? 0 : XD3_NOCOMPRESS
    );

    // If an error happened, print it here
    if (ret != 0) JAFFAR_THROW_RUNTIME("[Error] unexpected error while encoding differential compression. Probably maximum size increased: (%lu + %lu > %lu)", _outputDataBufferPos, *diffCount, _outputDataBufferSize);

    // Increasing output data position pointer
    _outputDataBufferPos += *diffCount;

    // Finally, increasing reference data position pointer
    _referenceDataBufferPos += inputDataSize;
  }

  size_t getReferenceDataBufferPos() const { return _referenceDataBufferPos; }

  private:

  const uint8_t* __restrict const _referenceDataBuffer;
  const size_t _referenceDataBufferSize;
  size_t _referenceDataBufferPos = 0;

  const bool _useZlib;
};

} // namespace serializer

} // namespace jaffarCommon