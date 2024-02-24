#pragma once

/**
 * @file differential.hpp
 * @brief Contains the differential data serializer
 */

#include "../../extern/xdelta3/xdelta3.h"
#include <stdexcept>
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

  inline void pushContiguous(const void* const __restrict inputData, const size_t inputDataSize) override
  {
    // Only perform memcpy if the output block is not null
    if (_outputDataBuffer != nullptr) memcpy(&_outputDataBuffer[_outputDataBufferPos], inputData, inputDataSize);

    // Moving output data pointer position
    _outputDataBufferPos += inputDataSize;

    // Making sure we do not exceed the maximum size estipulated
    if (_outputDataBufferPos > _outputDataBufferSize) throw std::runtime_error("Maximum output data position reached before contiguous serialization");

    // Moving reference data pointer position
    _referenceDataBufferPos += inputDataSize;
    if (_referenceDataBufferPos > _referenceDataBufferSize) throw std::runtime_error("[Error] Maximum reference data position exceeded on contiguous deserialization");
  }

  inline void push(const void* const __restrict inputData, const size_t inputDataSize) override
  {
    // If output data buffer is null, then we simply ignore differential data.
    if (_outputDataBuffer == nullptr) return;

    // Check that we don't exceed reference data size
    if (_referenceDataBufferPos + inputDataSize > _referenceDataBufferSize) throw std::runtime_error("[Error] Differential compression size exceeds reference data buffer size.");

    // Variable to store difference count 
    auto diffCount = (usize_t*)&_outputDataBuffer[_outputDataBufferPos];

    // Advancing position pointer to store the difference counter
    _outputDataBufferPos += sizeof(usize_t);

    // If we reached maximum output, stop here
    if (_outputDataBufferPos >= _outputDataBufferSize) throw std::runtime_error("[Error] Maximum output data position reached before differential encode");

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
    if (ret != 0) throw std::runtime_error("[Error] unexpected error while encoding differential compression.");

    // Increasing output data position pointer
    _outputDataBufferPos += *diffCount;

    // If exceeded size, report it
    if (_outputDataBufferPos > _outputDataBufferSize) throw std::runtime_error("[Error] Differential compression size exceeded output maximum size.");

    // Finally, increasing reference data position pointer
    _referenceDataBufferPos += inputDataSize;
  }

  size_t getReferenceSize() const { return _referenceDataBufferPos; }
  
  private:

  const uint8_t* __restrict const _referenceDataBuffer;
  const size_t _referenceDataBufferSize;
  size_t _referenceDataBufferPos = 0;

  const bool _useZlib;
};

} // namespace serializer

} // namespace jaffarCommon