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

/**
 * The differential deserialization class enables the decompression of a differential input buffer that, when applied to a reference data buffer,
 * produces the original source data. The decompression can be applied to different elements at different times. It also enables the use of
 * contiguous storage for elements that are not meant to be compressed.
 */
class Differential final : public deserializer::Base
{
public:
  /**
   * Default constructor for the differntial deserializer class
   *
   * @param[in] inputDataBuffer The input buffer from whence to read the input data
   * @param[in] inputDataBufferSize The size of the input buffer
   * @param[in] referenceDataBuffer The buffer from whence to read the reference data
   * @param[in] referenceDataBufferSize The size of the reference buffer
   * @param[in] useZlib Specifies whether to apply Zlib decompression before the differential decompression
   */
  Differential(const void* __restrict inputDataBuffer = nullptr, const size_t     inputDataBufferSize = std::numeric_limits<uint32_t>::max(),
               const void* __restrict referenceDataBuffer = nullptr, const size_t referenceDataBufferSize = std::numeric_limits<uint32_t>::max(), const bool useZlib = false)
      : deserializer::Base(inputDataBuffer, inputDataBufferSize)
      , _referenceDataBuffer((const uint8_t*)referenceDataBuffer)
      , _referenceDataBufferSize(referenceDataBufferSize)
      , _useZlib(useZlib)
  {
  }

  ~Differential() = default;

  __JAFFAR_COMMON_INLINE__ void popContiguous(void* const __restrict outputDataBuffer, const size_t outputDataSize) override
  {
    // Making sure we do not exceed the maximum size estipulated
    if (_inputDataBufferPos + outputDataSize > _inputDataBufferSize)
      JAFFAR_THROW_RUNTIME("Maximum input data position reached before contiguous deserialization of (%lu + %lu > %lu) bytes", _inputDataBufferPos, outputDataSize,
                           _inputDataBufferSize);
    if (_referenceDataBufferPos + outputDataSize > _referenceDataBufferSize)
      JAFFAR_THROW_RUNTIME("[Error] Maximum reference data position to be exceeded on contiguous deserialization (%lu + %lu > %lu)", _referenceDataBufferPos, outputDataSize,
                           _referenceDataBufferSize);

    // Only perform memcpy if the input block is not null
    if (_inputDataBuffer != nullptr && outputDataBuffer != nullptr) memcpy(outputDataBuffer, &_inputDataBuffer[_inputDataBufferPos], outputDataSize);

    // Moving input data pointer position
    _inputDataBufferPos += outputDataSize;

    // Moving reference data buffer position
    _referenceDataBufferPos += outputDataSize;
  }

  __JAFFAR_COMMON_INLINE__ void pop(void* const __restrict outputDataBuffer, const size_t outputDataSize) override
  {
    if (outputDataBuffer == nullptr || _inputDataBuffer == nullptr) return;

    // Reading differential outputDataBufferSize
    usize_t diffCount = *(usize_t*)&_inputDataBuffer[_inputDataBufferPos];

    // Size of differential buffer size
    const size_t differentialBufferSize = sizeof(usize_t);

    // If we reached maximum output, stop here
    if (_inputDataBufferPos + differentialBufferSize >= _inputDataBufferSize)
      JAFFAR_THROW_RUNTIME("[Error] Maximum input data position reached before differential buffer size decode (%lu + %lu > %lu)", _inputDataBufferPos, differentialBufferSize,
                           _inputDataBufferSize);

    // Advancing position pointer to store the difference outputDataBufferSizeer
    _inputDataBufferPos += differentialBufferSize;

    // If we reached maximum output, stop here
    if (_referenceDataBufferPos + outputDataSize > _referenceDataBufferSize)
      JAFFAR_THROW_RUNTIME("[Error] Maximum reference data position exceeded before differential decode (%lu + %lu > %lu)", _referenceDataBufferPos, outputDataSize,
                           _referenceDataBufferSize);

    // Encoding differential
    usize_t output_size;
    int     ret = xd3_decode_memory(&_inputDataBuffer[_inputDataBufferPos], diffCount, &_referenceDataBuffer[_referenceDataBufferPos], outputDataSize, (uint8_t*)outputDataBuffer,
                                    &output_size, outputDataSize, _useZlib ? 0 : XD3_NOCOMPRESS);

    // If an error happened, print it here
    if (ret != 0)
      JAFFAR_THROW_RUNTIME(
          "[Error] unexpected error while decoding differential decompression. Probably maximum input data position reached after differential decode (%lu + %lu > %lu)",
          _inputDataBufferPos, diffCount, _inputDataBufferSize);

    // Increasing output data position pointer
    _inputDataBufferPos += diffCount;

    // Increasing the number of differential bytes processed
    _differentialBytesCount += diffCount;

    // Finally, increasing reference data position pointer
    _referenceDataBufferPos += outputDataSize;
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
   * This is useful to make sure the differential part of the deserialization is not exceeded
   *
   * @return The number of bytes used in differential decompression
   */
  size_t getDifferentialBytesCount() const { return _differentialBytesCount; }

private:
  /**
   *  The internally-stored reference data buffer
   */
  const uint8_t* __restrict const _referenceDataBuffer;

  /**
   *  The reference data buffer size, as provided by the used
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

} // namespace deserializer

} // namespace jaffarCommon