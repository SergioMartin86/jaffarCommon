#include "gtest/gtest.h"
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/serializers/differential.hpp>
#include <jaffarCommon/deserializers/differential.hpp>

using namespace jaffarCommon;

TEST(contiguous, serialization)
{
  std::string input1 = "Hello,";
  std::string input2 = "World!";

  const void* input1Buffer = input1.data();
  const size_t input1BufferSize = input1.size();
  const void* input2Buffer = input2.data();
  const size_t input2BufferSize = input2.size();

  const size_t outputBufferSize = 256;
  auto outputBuffer = calloc(1, outputBufferSize);

  serializer::Contiguous s(outputBuffer, outputBufferSize);
  ASSERT_THROW(s.push(nullptr, outputBufferSize + 1), std::runtime_error);
  ASSERT_NO_THROW(s.push(input1Buffer, input1BufferSize));
  ASSERT_NO_THROW(s.push(input2Buffer, input2BufferSize));

  std::string output((const char*)outputBuffer);
  ASSERT_EQ(input1 + input2, output);

  free(outputBuffer);
}

TEST(contiguous, deserialization)
{
  std::string input = "Hello, World!";

  const void* inputBuffer = input.data();
  const size_t inputBufferSize = input.size();

  const size_t output1BufferSize = 6;
  auto output1Buffer = calloc(1, output1BufferSize);
  const size_t output2BufferSize = inputBufferSize - output1BufferSize;
  auto output2Buffer = calloc(1, output2BufferSize);

  deserializer::Contiguous d(inputBuffer, inputBufferSize);
  ASSERT_NO_THROW(d.pop(output1Buffer, output1BufferSize));
  ASSERT_NO_THROW(d.pop(output2Buffer, output2BufferSize));
  ASSERT_THROW(d.pop(nullptr, 1), std::runtime_error);
  
  std::string output1((const char*)output1Buffer);
  std::string output2((const char*)output2Buffer);
  
  ASSERT_EQ(input, output1 + output2);

  free(output1Buffer);
  free(output2Buffer);
}

TEST(contiguous, serializerGetters)
{
  std::string input1 = "Hello,";
  std::string input2 = "World!";

  const void* input1Buffer = input1.data();
  const size_t input1BufferSize = input1.size();
  const void* input2Buffer = input2.data();
  const size_t input2BufferSize = input2.size();

  serializer::Contiguous s(nullptr);

  ASSERT_NO_THROW(s.push(input1Buffer, input1BufferSize));
  ASSERT_EQ(s.getOutputSize(), input1BufferSize);

  ASSERT_NO_THROW(s.push(input2Buffer, input2BufferSize));
  ASSERT_EQ(s.getOutputSize(), input1BufferSize + input2BufferSize);

  ASSERT_EQ(s.getOutputDataBuffer(), nullptr);
}

TEST(contiguous, deserializerGetters)
{
  std::string input = "Hello, World!";

  const void* inputBuffer = input.data();
  const size_t inputBufferSize = input.size();

  const size_t output1BufferSize = 6;
  const size_t output2BufferSize = inputBufferSize - output1BufferSize;

  deserializer::Contiguous d(inputBuffer, inputBufferSize);

  ASSERT_NO_THROW(d.pop(nullptr, output1BufferSize));
  ASSERT_EQ(d.getInputSize(), output1BufferSize);
  
  ASSERT_NO_THROW(d.pop(nullptr, output2BufferSize));
  ASSERT_EQ(d.getInputSize(), output1BufferSize + output2BufferSize);

  ASSERT_EQ(d.getInputDataBuffer(), inputBuffer);
}

TEST(differential, fullCycleNoZlib)
{
  const std::string reference = "Hello, World!"; // Reference Data
  const std::string input1    = "Hallo,"; // 1 Difference
  const std::string input2    = " "; // Contigously serialized
  const std::string input3    = "Yerld!"; // 2 Differences

  bool useZlib = false;
  const void* input1Buffer   = input1.data();
  const size_t input1BufferSize = input1.size();
  const void* input2Buffer   = input2.data();
  const size_t input2BufferSize = input2.size();
  const void* input3Buffer   = input3.data();
  const size_t input3BufferSize = input3.size();

  const void* referenceBuffer = reference.data();
  const size_t referenceBufferSize = reference.size();

  ASSERT_EQ(input1BufferSize + input2BufferSize + input3BufferSize, referenceBufferSize);

  // Serializing ----------------------

  const size_t serializationOutputBufferSize = 256;
  auto serializationOutputBuffer = calloc(1, serializationOutputBufferSize);

  serializer::Differential sSmallReferenceDataBuffer(serializationOutputBuffer, serializationOutputBufferSize, referenceBuffer, 0, useZlib);
  ASSERT_THROW(sSmallReferenceDataBuffer.push(input1Buffer, 1), std::runtime_error);
  ASSERT_THROW(sSmallReferenceDataBuffer.pushContiguous(input1Buffer, 1), std::runtime_error);

  serializer::Differential sSmallOutputDataBuffer(serializationOutputBuffer, 0, referenceBuffer, referenceBufferSize, useZlib);
  ASSERT_THROW(sSmallOutputDataBuffer.push(input1Buffer, 1), std::runtime_error);
  ASSERT_THROW(sSmallOutputDataBuffer.pushContiguous(input1Buffer, 1), std::runtime_error);

  serializer::Differential sSmallOutputDataBuffer2(serializationOutputBuffer, 5, referenceBuffer, referenceBufferSize, useZlib);
  ASSERT_THROW(sSmallOutputDataBuffer2.push(input1Buffer, 10), std::runtime_error);
  ASSERT_THROW(sSmallOutputDataBuffer2.pushContiguous(input1Buffer, 10), std::runtime_error);

  serializer::Differential s(serializationOutputBuffer, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, useZlib);

  size_t currentReferenceBufferPosition = s.getReferenceDataBufferPos();
  ASSERT_EQ(currentReferenceBufferPosition, 0);

  size_t currentOutputSize = s.getReferenceDataBufferPos();
  ASSERT_EQ(currentOutputSize, 0);
  ASSERT_EQ(s.getDifferentialBytesCount(), 0);
  ASSERT_THROW(s.push(input3Buffer, serializationOutputBufferSize + 1), std::runtime_error);
  ASSERT_NO_THROW(s.push(input1Buffer, input1BufferSize));
  ASSERT_GT(s.getDifferentialBytesCount(), 0);
  
  ASSERT_EQ(s.getReferenceDataBufferPos(), currentReferenceBufferPosition + input1BufferSize);
  currentReferenceBufferPosition = s.getReferenceDataBufferPos();

  ASSERT_GT(s.getOutputSize(), currentOutputSize);
  currentOutputSize = s.getOutputSize();

  ASSERT_NO_THROW(s.pushContiguous(input2Buffer, input2BufferSize));

  ASSERT_EQ(s.getReferenceDataBufferPos(), currentReferenceBufferPosition + input2BufferSize);
  currentReferenceBufferPosition = s.getReferenceDataBufferPos();

  ASSERT_GT(s.getOutputSize(), currentOutputSize);
  currentOutputSize = s.getOutputSize();

  ASSERT_NO_THROW(s.push(input3Buffer, input3BufferSize));
  
  ASSERT_EQ(s.getReferenceDataBufferPos(), currentReferenceBufferPosition + input3BufferSize);
  currentReferenceBufferPosition = s.getReferenceDataBufferPos();

  ASSERT_GT(s.getOutputSize(), currentOutputSize);
  currentOutputSize = s.getOutputSize();

  // Deserializing -------------

  const size_t deserializationOutput1BufferSize = input1BufferSize;
  auto deserializationOutput1Buffer = calloc(1, input1BufferSize);
  const size_t deserializationOutput2BufferSize = input2BufferSize;
  auto deserializationOutput2Buffer = calloc(1, input2BufferSize);
  const size_t deserializationOutput3BufferSize = input3BufferSize;
  auto deserializationOutput3Buffer = calloc(1, input3BufferSize);

  const auto deserializationInputBuffer = serializationOutputBuffer;
  const auto deserializationInputBufferSize = currentOutputSize;

  deserializer::Differential dSmallReferenceDataBuffer(deserializationInputBuffer, deserializationInputBufferSize, referenceBuffer, 0, useZlib);
  ASSERT_THROW(dSmallReferenceDataBuffer.pop(deserializationOutput1Buffer, 1), std::runtime_error);
  ASSERT_THROW(dSmallReferenceDataBuffer.popContiguous(deserializationOutput1Buffer, 1), std::runtime_error);

  deserializer::Differential dSmallInputDataBuffer(deserializationInputBuffer, 0, referenceBuffer, referenceBufferSize, useZlib);
  ASSERT_THROW(dSmallInputDataBuffer.pop(deserializationOutput1Buffer, 1), std::runtime_error);
  ASSERT_THROW(dSmallInputDataBuffer.popContiguous(deserializationOutput1Buffer, 1), std::runtime_error);

  deserializer::Differential dSmallInputDataBuffer2(deserializationInputBuffer, 5, referenceBuffer, referenceBufferSize, useZlib);
  ASSERT_THROW(dSmallInputDataBuffer2.pop(deserializationOutput1Buffer, 1), std::runtime_error);
  ASSERT_THROW(dSmallInputDataBuffer2.popContiguous(deserializationOutput1Buffer, 10), std::runtime_error);

  deserializer::Differential d(deserializationInputBuffer, deserializationInputBufferSize, referenceBuffer, referenceBufferSize, useZlib);

  ASSERT_THROW(s.push(input3Buffer, deserializationOutput1BufferSize + 1), std::runtime_error);
  ASSERT_EQ(d.getDifferentialBytesCount(), 0);
  ASSERT_NO_THROW(d.pop(deserializationOutput1Buffer, deserializationOutput1BufferSize));
  ASSERT_GT(d.getDifferentialBytesCount(), 0);
  ASSERT_NO_THROW(d.popContiguous(deserializationOutput2Buffer, deserializationOutput2BufferSize));
  ASSERT_NO_THROW(d.pop(deserializationOutput3Buffer, deserializationOutput3BufferSize));

  ASSERT_EQ(input1, std::string((const char*)deserializationOutput1Buffer));
  ASSERT_EQ(input2, std::string((const char*)deserializationOutput2Buffer));
  ASSERT_EQ(input3, std::string((const char*)deserializationOutput3Buffer));

  free(deserializationOutput1Buffer);
  free(deserializationOutput2Buffer);
  free(deserializationOutput3Buffer);
  free(serializationOutputBuffer);
}

TEST(differential, fullCycleZlib)
{
  const std::string reference = "fapinfaepnepanpeaincpaeijiaepraefipeapfnapenfapenfpaeinfpaeinfcaenfeainfcaeonfaeocinfaeiox"; // Reference Data
  const std::string input     = "apfaepmaepmcaeiopmccoa<emccoaemcocaenoeacnfocaenfocnaefocnaeocfnaeocinffocaeijraecifja<eaf"; 

  const void* inputBuffer   = input.data();
  const size_t inputBufferSize = input.size();

  const void* referenceBuffer = reference.data();
  const size_t referenceBufferSize = reference.size();

  // Serializing ----------------------

  const size_t serializationOutputBufferSize = 4096;
  auto serializationOutputBuffer = calloc(1, serializationOutputBufferSize);
  auto serializationOutputBufferNoZlib = calloc(1, serializationOutputBufferSize);

  serializer::Differential sZlib(serializationOutputBuffer, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, true);
  serializer::Differential sNoZlib(serializationOutputBufferNoZlib, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, false);

  ASSERT_NO_THROW(sNoZlib.push(inputBuffer, inputBufferSize));
  ASSERT_NO_THROW(sZlib.push(inputBuffer, inputBufferSize));

  size_t serializationOutputBufferActualSize = sZlib.getOutputSize();
  size_t serializationOutputBufferNoZlibActualSize = sNoZlib.getOutputSize();
  ASSERT_LT(serializationOutputBufferActualSize, serializationOutputBufferNoZlibActualSize);

  // Deserializing -------------

  const size_t deserializationOutputBufferSize = inputBufferSize;
  auto deserializationOutputBuffer = calloc(1, deserializationOutputBufferSize);
  auto deserializationOutputBufferNoZlib = calloc(1, deserializationOutputBufferSize);

  const auto deserializationInputBuffer = serializationOutputBuffer;
  const auto deserializationInputBufferNoZlib = serializationOutputBufferNoZlib;

  deserializer::Differential d(deserializationInputBuffer, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, true);
  deserializer::Differential dNoZlib(deserializationInputBufferNoZlib, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, false);

  ASSERT_NO_THROW(d.pop(deserializationOutputBuffer, inputBufferSize));
  ASSERT_NO_THROW(dNoZlib.pop(deserializationOutputBufferNoZlib, inputBufferSize));

  ASSERT_EQ(input, std::string((const char*)deserializationOutputBuffer));
  ASSERT_EQ(input, std::string((const char*)deserializationOutputBufferNoZlib));

  free(deserializationOutputBuffer);
  free(deserializationOutputBufferNoZlib);
  free(serializationOutputBuffer);
  free(serializationOutputBufferNoZlib);
}

TEST(differential, nullBuffer)
{
  const std::string reference = "Hello, World!"; // Reference Data
  const std::string input1    = "Hallo,"; // 1 Difference
  const std::string input2    = " "; // Contigously serialized
  const std::string input3    = "Yerld!"; // 2 Differences

  bool useZlib = false;
  const void* input1Buffer   = input1.data();
  const size_t input1BufferSize = input1.size();
  const void* input2Buffer   = input2.data();
  const size_t input2BufferSize = input2.size();
  const void* input3Buffer   = input3.data();
  const size_t input3BufferSize = input3.size();

  const void* referenceBuffer = reference.data();
  const size_t referenceBufferSize = reference.size();

  ASSERT_EQ(input1BufferSize + input2BufferSize + input3BufferSize, referenceBufferSize);

  // Serializing ----------------------

  const size_t serializationOutputBufferSize = 256;
  serializer::Differential s(nullptr, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, useZlib);

  ASSERT_NO_THROW(s.push(input1Buffer, input1BufferSize));
  ASSERT_NO_THROW(s.pushContiguous(input2Buffer, input2BufferSize));
  ASSERT_NO_THROW(s.push(input3Buffer, input3BufferSize));
  ASSERT_GT(s.getOutputSize(), 0);

  // Deserializing -------------
  deserializer::Differential d(nullptr, serializationOutputBufferSize, referenceBuffer, referenceBufferSize, useZlib);

  ASSERT_NO_THROW(d.pop(nullptr, input1BufferSize));
  ASSERT_NO_THROW(d.popContiguous(nullptr, input2BufferSize));
  ASSERT_NO_THROW(d.pop(nullptr, input3BufferSize));
  ASSERT_GT(d.getInputSize(), 0);
}