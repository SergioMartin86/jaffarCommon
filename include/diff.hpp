#pragma once

/**
 * @file diff.hpp
 * @brief Contains common functions related to serialization and deserialization when using differential compression
 */

#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <limits>
#include "../extern/xdelta3/xdelta3.h"

namespace jaffarCommon
{

inline void serializeContiguousData(
  const void* __restrict__ inputData,
  const size_t inputDataSize,
  uint8_t* __restrict__ outputData,
  size_t* outputDataPos,
  const size_t outputDataMaxSize,
  size_t* referenceDataPos,
  const size_t referenceDataMaxSize)
{
  // Only perform memcpy if the input block is not null
  if (outputData != nullptr) memcpy(&outputData[*outputDataPos], inputData, inputDataSize);

  // Moving output data pointer position
  *outputDataPos += inputDataSize;

  // Making sure we do not exceed the maximum size estipulated
  if (*outputDataPos > outputDataMaxSize) throw std::runtime_error("Maximum output data position reached before contiguous serialization");

  // Moving reference data pointer position
  if (referenceDataPos != nullptr)
  {
    *referenceDataPos += inputDataSize;
    if (*referenceDataPos > referenceDataMaxSize) throw std::runtime_error("[Error] Maximum reference data position exceeded on contiguous deserialization");
  } 
}

inline void deserializeContiguousData(
  void* __restrict__ outputData,
  const size_t outputDataSize,
  const uint8_t* __restrict__ inputData,
  size_t* inputDataPos,
  const size_t inputDataMaxSize,
  size_t* referenceDataPos,
  const size_t referenceDataMaxSize)
{
  // Only perform memcpy if the input block is not null
  if (outputData != nullptr) memcpy(outputData, &inputData[*inputDataPos], outputDataSize);

  // Moving input data pointer position
  *inputDataPos += outputDataSize;

  // Making sure we do not exceed the maximum size estipulated
  if (*inputDataPos > inputDataMaxSize) throw std::runtime_error("Maximum input data position reached before contiguous deserialization");

  // Moving reference data position
  if (referenceDataPos != nullptr)
  {
    *referenceDataPos += outputDataSize;
    if (*referenceDataPos > referenceDataMaxSize) throw std::runtime_error("[Error] Maximum reference data position exceeded on contiguous deserialization");
  } 
}

inline void serializeDifferentialData(
  const uint8_t* __restrict__ inputData,
  const size_t inputDataSize,
  uint8_t* __restrict__ outputData,
  size_t* outputDataPos,
  const size_t outputDataMaxSize,
  const uint8_t* __restrict__ referenceData = nullptr,
  size_t* referenceDataPos = 0,
  const size_t referenceDataMaxSize = 0,
  const bool useZlib = false
  )
{
  // Check that we don't exceed reference data size
  if (*referenceDataPos + inputDataSize > referenceDataMaxSize) throw std::runtime_error("[Error] Differential compression size exceeded reference data maximum size.");

  // Only perform compression if input is not null
  if (outputData != nullptr)
  {
    // Variable to store difference count 
    auto diffCount = (usize_t*)&outputData[*outputDataPos];

    // Advancing position pointer to store the difference counter
    *outputDataPos += sizeof(usize_t);

    // If we reached maximum output, stop here
    if (*outputDataPos >= outputDataMaxSize) throw std::runtime_error("[Error] Maximum output data position reached before differential encode");

    // Encoding differential
    int ret = xd3_encode_memory(
      inputData,
      inputDataSize,
      &referenceData[*referenceDataPos],
      inputDataSize,
      &outputData[*outputDataPos],
      diffCount,
      outputDataMaxSize - *outputDataPos,
      useZlib ? 0 : XD3_NOCOMPRESS
    );

    // If an error happened, print it here
    if (ret != 0) throw std::runtime_error("[Error] unexpected error while encoding differential compression.");

    // Increasing output data position pointer
    *outputDataPos += *diffCount;

    // If exceeded size, report it
    if (*outputDataPos > outputDataMaxSize) throw std::runtime_error("[Error] Differential compression size exceeded output maximum size.");
  }

  // Finally, increasing reference data position pointer
  *referenceDataPos += inputDataSize;
}

inline void deserializeDifferentialData(
  uint8_t* __restrict__ outputData,
  const size_t outputDataSize,
  const uint8_t* __restrict__ inputData,
  size_t* inputDataPos,
  const size_t inputDataMaxSize,
  const uint8_t* __restrict__ referenceData = nullptr,
  size_t* referenceDataPos = 0,
  const size_t referenceDataMaxSize = 0,
  const bool useZlib = false
  )
{
  // Reading differential count
  usize_t diffCount = *(usize_t*) &inputData[*inputDataPos];

  // Advancing position pointer to store the difference counter
  *inputDataPos += sizeof(usize_t);

  // If we reached maximum output, stop here
  if (*inputDataPos >= inputDataMaxSize)  throw std::runtime_error("[Error] Maximum input data position reached before differential decode");

  // If we reached maximum output, stop here
  if (*referenceDataPos + outputDataSize > referenceDataMaxSize) throw std::runtime_error("[Error] Maximum reference data position exceeded before differential decode");

  // Encoding differential
  usize_t output_size;
  int ret = xd3_decode_memory(
    &inputData[*inputDataPos],
    diffCount,
    &referenceData[*referenceDataPos],
    outputDataSize,
    outputData,
    &output_size,
    outputDataSize,
    useZlib ? 0 : XD3_NOCOMPRESS
  );

  // If an error happened, print it here
  if (ret != 0) throw std::runtime_error("[Error] unexpected error while decoding differential decompression.");

  // Increasing output data position pointer
  *inputDataPos += diffCount;

  // If we reached maximum output, stop here
  if (*inputDataPos >= inputDataMaxSize) throw std::runtime_error("[Error] Maximum input data position reached after differential decode");

  // Finally, increasing reference data position pointer
  *referenceDataPos += outputDataSize;
}

class Serializer final
{
  public:

  Serializer(
    void* __restrict__ outputDataBuffer = nullptr, 
    const size_t outputDataBufferSize = std::numeric_limits<uint32_t>::max(),
    const void* __restrict__ referenceDataBuffer = nullptr,
    const size_t referenceDataBufferSize = 0,
    const bool useZlib = false
  ) :
   _outputDataBuffer((uint8_t*)outputDataBuffer),
   _outputDataBufferSize(outputDataBufferSize),
   _referenceDataBuffer((const uint8_t*)referenceDataBuffer),
   _referenceDataBufferSize(referenceDataBufferSize),
   _useZlib(useZlib)
  {

  }

  ~Serializer() = default;

  void pushData(const void* const __restrict__ inputData, const size_t inputDataSize)
  {
    if (_referenceDataBuffer == nullptr) pushContiguousData(inputData, inputDataSize);
    if (_referenceDataBuffer != nullptr) pushDifferentialData(inputData, inputDataSize);
  }

  void pushContiguousData(const void* const __restrict__ inputData, const size_t inputDataSize)
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

  void pushDifferentialData(const void* const __restrict__ inputData, const size_t inputDataSize)
  {
    // If output data buffer is null, then we simply ignore differential data.
    if (_outputDataBuffer != nullptr) return;

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

  size_t getOutputSize() const { return _outputDataBufferPos; }
  
  private:

  uint8_t* __restrict__ const _outputDataBuffer;
  const size_t _outputDataBufferSize;
  size_t _outputDataBufferPos = 0;

  const uint8_t* __restrict__ const _referenceDataBuffer;
  const size_t _referenceDataBufferSize;
  size_t _referenceDataBufferPos = 0;

  const bool _useZlib;
};


}