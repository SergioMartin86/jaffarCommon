#pragma once

/**
 * @file differential.hpp
 * @brief Contains the differential data serializer
 */

#include <cstring>
#include <limits>
#include <xdelta3/xdelta3.h>
#include "base.hpp"

namespace jaffarCommon
{

namespace serializer
{

/**
 * The differential serialization class enables the compression of a input elements that, when applied to a reference data buffer,
 * produces the a compressed output. Well used, it produces an output that is smaller than the original data. The original data
 * can be recovered later by comparing the output to the reference data.
 *
 * The compression can be applied to different elements at different times. It also enables the use of contiguous storage for elements
 * that are not meant to be compressed.
 */
class Differential final : public serializer::Base
{
  public:

  /**
   * Default constructor for the differntial deserializer class
   *
   * @param[in] outputDataBuffer The output buffer onto which to write the serialized data
   * @param[in] outputDataBufferSize The size of the output buffer
   * @param[in] referenceDataBuffer The buffer from whence to read the reference data
   * @param[in] referenceDataBufferSize The size of the reference buffer
   * @param[in] useZlib Specifies whether to apply Zlib compression after the differential compression
   */
  Differential(void *__restrict outputDataBuffer          = nullptr,
               const size_t outputDataBufferSize          = std::numeric_limits<uint32_t>::max(),
               const void *__restrict referenceDataBuffer = nullptr,
               const size_t referenceDataBufferSize       = std::numeric_limits<uint32_t>::max(),
               const bool   useZlib                       = false)
    : serializer::Base(outputDataBuffer, outputDataBufferSize)
    , _referenceDataBuffer((const uint8_t *)referenceDataBuffer)
    , _referenceDataBufferSize(referenceDataBufferSize)
    , _useZlib(useZlib)
  {}

  ~Differential() = default;

  __INLINE__ void pushContiguous(const void *const __restrict inputData = nullptr, const size_t inputDataSize = 0) override
  {
    // Only perform memcpy if the output block is not null
    if (_outputDataBuffer != nullptr && inputData != nullptr) memcpy(&_outputDataBuffer[_outputDataBufferPos], inputData, inputDataSize);

    // Making sure we do not exceed the maximum size estipulated
    if (_outputDataBufferPos + inputDataSize > _outputDataBufferSize)
      JAFFAR_THROW_RUNTIME("Maximum output data position reached before contiguous serialization (%lu + %lu > %lu)", _outputDataBufferPos, inputDataSize, _outputDataBufferSize);
    if (_referenceDataBufferPos + inputDataSize > _referenceDataBufferSize)
      JAFFAR_THROW_RUNTIME(
        "[Error] Maximum reference data position exceeded on contiguous deserialization (%lu + %lu > %lu)", _referenceDataBufferPos, inputDataSize, _referenceDataBufferSize);

    // Moving output data pointer position
    _outputDataBufferPos += inputDataSize;

    // Moving reference data pointer position
    _referenceDataBufferPos += inputDataSize;
  }

  __INLINE__ void push(const void *const __restrict inputData, const size_t inputDataSize) override
  {
    // If output data buffer is null, then we simply ignore differential data.
    if (_outputDataBuffer == nullptr || inputData == nullptr) return;

    // Check that we don't exceed reference data size
    if (_referenceDataBufferPos + inputDataSize > _referenceDataBufferSize)
      JAFFAR_THROW_RUNTIME(
        "[Error] Differential compression size exceeds reference data buffer size (%lu + %lu > %lu)", _referenceDataBufferPos, inputDataSize, _referenceDataBufferSize);

    // Variable to store difference count
    auto diffCount = (usize_t *)&_outputDataBuffer[_outputDataBufferPos];

    // Size of differential buffer size
    const size_t differentialBufferSize = sizeof(usize_t);

    // If we reached maximum output, stop here
    if (_outputDataBufferPos + differentialBufferSize >= _outputDataBufferSize)
      JAFFAR_THROW_RUNTIME(
        "[Error] Maximum output data position reached before differential encode  (%lu + %lu > %lu)", _outputDataBufferPos, differentialBufferSize, _outputDataBufferSize);

    // Advancing position pointer to store the difference counter
    _outputDataBufferPos += differentialBufferSize;

    // Encoding differential
    int ret = xd3_encode_memory((const uint8_t *)inputData,
                                inputDataSize,
                                &_referenceDataBuffer[_referenceDataBufferPos],
                                inputDataSize,
                                &_outputDataBuffer[_outputDataBufferPos],
                                diffCount,
                                _outputDataBufferSize - _outputDataBufferPos,
                                _useZlib ? 0 : XD3_NOCOMPRESS);

    // If an error happened, print it here
    if (ret != 0)
      JAFFAR_THROW_RUNTIME("[Error] unexpected error while encoding differential compression. Probably maximum size increased: (%lu + %lu > %lu)",
                           _outputDataBufferPos,
                           *diffCount,
                           _outputDataBufferSize);

    // Increasing output data position pointer
    _outputDataBufferPos += *diffCount;

    // Increasing the number of differential bytes processed
    _differentialBytesCount += *diffCount;

    // Finally, increasing reference data position pointer
    _referenceDataBufferPos += inputDataSize;
  }

  /**
   * Get the position of the reference buffer header.
   *
   * @return The position of the reference buffer header. This value represents the size of the reference data at the end of the deserialization process
   */
  size_t getReferenceDataBufferPos() const { return _referenceDataBufferPos; }

  /**
   * Gets the number of differential bytes included in the serialized output
   *
   * This is useful to make sure the differential part of the serialization is not exceeded
   *
   * @return The number of bytes used in differential compression
   */
  size_t getDifferentialBytesCount() const { return _differentialBytesCount; }

  private:

  /**
   *  The internally-stored reference data buffer
   */
  const uint8_t *__restrict const _referenceDataBuffer;

  /**
   *  The maximum size of the input data buffer
   */
  const size_t _referenceDataBufferSize;

  /**
   *  The current position of the reference data buffer header
   */
  size_t _referenceDataBufferPos = 0;

  /**
   *  Differential bytes count
   */
  size_t _differentialBytesCount = 0;

  /**
   *  Stores whether to use Zlib compression after the differential compression
   */
  const bool _useZlib;
};

} // namespace serializer

} // namespace jaffarCommon