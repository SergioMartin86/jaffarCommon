#include "gtest/gtest.h"
#include "../include/serializers/contiguous.hpp"
#include "../include/deserializers/contiguous.hpp"
#include "../include/serializers/differential.hpp"
#include "../include/deserializers/differential.hpp"

using namespace jaffarCommon;

TEST(contiguous, serialization)
{
  std::string input1 = "Hello,";
  std::string input2 = "World!";

  const uint8_t* input1Buffer = (const uint8_t*) input1.data();
  const size_t input1BufferSize = input1.size();
  const uint8_t* input2Buffer = (const uint8_t*) input2.data();
  const size_t input2BufferSize = input2.size();

  const size_t outputBufferSize = 256;
  auto outputBuffer = (uint8_t*) calloc(1, outputBufferSize);

  serializer::Contiguous s(outputBuffer, outputBufferSize);
  EXPECT_THROW(s.push(nullptr, outputBufferSize + 1), std::runtime_error);
  EXPECT_NO_THROW(s.push(input1Buffer, input1BufferSize));
  EXPECT_NO_THROW(s.push(input2Buffer, input2BufferSize));

  std::string output((const char*)outputBuffer);
  EXPECT_EQ(input1 + input2, output);

  free(outputBuffer);
}

TEST(contiguous, deserialization)
{
  std::string input = "Hello, World!";

  const uint8_t* inputBuffer = (const uint8_t*) input.data();
  const size_t inputBufferSize = input.size();

  const size_t output1BufferSize = 6;
  auto output1Buffer = (uint8_t*) calloc(1, output1BufferSize);
  const size_t output2BufferSize = inputBufferSize - output1BufferSize;
  auto output2Buffer = (uint8_t*) calloc(1, output2BufferSize);

  deserializer::Contiguous d(inputBuffer, inputBufferSize);
  EXPECT_NO_THROW(d.pop(output1Buffer, output1BufferSize));
  EXPECT_NO_THROW(d.pop(output2Buffer, output2BufferSize));
  EXPECT_THROW(d.pop(nullptr, 1), std::runtime_error);
  
  std::string output1((const char*)output1Buffer);
  std::string output2((const char*)output2Buffer);
  
  EXPECT_EQ(input, output1 + output2);

  free(output1Buffer);
  free(output2Buffer);
}

TEST(contiguous, serializerGetters)
{
  std::string input1 = "Hello,";
  std::string input2 = "World!";

  const uint8_t* input1Buffer = (const uint8_t*) input1.data();
  const size_t input1BufferSize = input1.size();
  const uint8_t* input2Buffer = (const uint8_t*) input2.data();
  const size_t input2BufferSize = input2.size();

  serializer::Contiguous s(nullptr);
  EXPECT_NO_THROW(s.push(input1Buffer, input1BufferSize));
  EXPECT_NO_THROW(s.push(input2Buffer, input2BufferSize));

  EXPECT_EQ(s.getOutputSize(), input1BufferSize + input2BufferSize);
  EXPECT_EQ(s.getOutputDataBuffer(), nullptr);
}
