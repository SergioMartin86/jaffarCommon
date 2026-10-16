#include "gtest/gtest.h"
#include <jaffarCommon/json.hpp>

using namespace jaffarCommon::json;

TEST(json, badObject)
{
  object input = std::vector<object>();

  EXPECT_THROW(getString(input, "Entry"), std::logic_error);
  EXPECT_THROW(getObject(input, "Entry"), std::logic_error);
  EXPECT_THROW(getArray<size_t>(input, "Entry"), std::logic_error);
  EXPECT_THROW(getNumber<size_t>(input, "Entry"), std::logic_error);
  EXPECT_THROW(getBoolean(input, "Entry"), std::logic_error);
}

TEST(json, badEntry)
{
  object input = object::parse("{ \"Entry\": { } }");

  EXPECT_THROW(getString(input, "Bad Entry"), std::logic_error);
  EXPECT_THROW(getObject(input, "Bad Entry"), std::logic_error);
  EXPECT_THROW(getArray<size_t>(input, "Bad Entry"), std::logic_error);
  EXPECT_THROW(getNumber<size_t>(input, "Bad Entry"), std::logic_error);
  EXPECT_THROW(getBoolean(input, "Bad Entry"), std::logic_error);
}

TEST(json, badType)
{
  object input = object::parse("{ \"Entry\": 1 }");

  EXPECT_THROW(getString(input, "Entry"), std::logic_error);
  EXPECT_THROW(getObject(input, "Entry"), std::logic_error);
  EXPECT_THROW(getArray<size_t>(input, "Entry"), std::logic_error);
  EXPECT_THROW(getBoolean(input, "Entry"), std::logic_error);

  input = object::parse("{ \"Entry\": \"Hello\" }");
  EXPECT_THROW(getNumber<size_t>(input, "Entry"), std::logic_error);
}

TEST(json, string)
{
  std::string result = "";
  std::string expected = "Hello, World!";
  object input = object::parse(std::string("{ \"Entry\": \"") + expected + std::string("\" }"));
  EXPECT_NO_THROW(result = getString(input, "Entry"));
  EXPECT_EQ(result, expected);
}

TEST(json, object)
{
  object input = object::parse(std::string("{ \"Object\": { \"Entry\": \"Hello, World!\" } }"));
  object result;
  EXPECT_NO_THROW(result = getObject(input, "Object"));
  EXPECT_TRUE(result.is_object());
}

TEST(json, arrayNumber)
{
  object input = object::parse(std::string("{ \"Array\": [0,1,2,3] }"));
  std::vector<int> result;
  EXPECT_NO_THROW(result = getArray<int>(input, "Array"));
  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], 0);
  EXPECT_EQ(result[1], 1);
  EXPECT_EQ(result[2], 2);
  EXPECT_EQ(result[3], 3);
}

TEST(json, arrayString)
{
  object input = object::parse(std::string("{ \"Array\": [ \"Hello,\",\" \", \"World!\" ] }"));
  std::vector<std::string> result;
  EXPECT_NO_THROW(result = getArray<std::string>(input, "Array"));
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], "Hello,");
  EXPECT_EQ(result[1], " ");
  EXPECT_EQ(result[2], "World!");
}

TEST(json, number)
{
  int result = 0;
  object input = object::parse(std::string("{ \"Number\": 42 }"));
  EXPECT_NO_THROW(result = getNumber<int>(input, "Number"));
  EXPECT_EQ(result, 42);
}

TEST(json, boolean)
{
  bool result = false;
  object input = object::parse(std::string("{ \"Boolean\": true }"));
  EXPECT_NO_THROW(result = getBoolean(input, "Boolean"));
  EXPECT_EQ(result, true);
}
TEST(json, integerRange)
{
  object input;
  input["Negative"]   = -1;
  input["Large"]      = 300;
  input["Fraction"]   = 2.5;
  input["Integral"]   = 64.0;
  input["Max"]        = 255;
  input["MinSigned"]  = -128;

  // Values that do not fit the requested integer type are rejected instead of wrapping or truncating
  EXPECT_THROW(getNumber<size_t>(input, "Negative"), std::logic_error);
  EXPECT_THROW(getNumber<uint8_t>(input, "Large"), std::logic_error);
  EXPECT_THROW(getNumber<int>(input, "Fraction"), std::logic_error);
  EXPECT_THROW(getNumber<int8_t>(input, "Max"), std::logic_error);

  // Values that fit are returned unchanged (an integral float included); floating types take anything
  EXPECT_EQ(getNumber<ssize_t>(input, "Negative"), -1);
  EXPECT_EQ(getNumber<uint8_t>(input, "Max"), 255);
  EXPECT_EQ(getNumber<int8_t>(input, "MinSigned"), -128);
  EXPECT_EQ(getNumber<size_t>(input, "Integral"), 64);
  EXPECT_EQ(getNumber<float>(input, "Fraction"), 2.5f);
  EXPECT_EQ(getNumber<double>(input, "Negative"), -1.0);
}

TEST(json, keyReadRecorder)
{
  object input;
  input["A"] = 1;
  input["B"] = "two";
  input["C"] = 3.0;
  object other;
  other["A"] = 5;

  {
    KeyReadRecorder recorder(input);
    getNumber<int>(input, "A");
    getString(input, "B");
    getNumber<int>(other, "A"); // another object: not recorded
    EXPECT_EQ(recorder.getKeys(), (std::set<std::string>{"A", "B"}));

    // Recorders nest; the outer one resumes afterwards
    {
      KeyReadRecorder inner(other);
      getNumber<int>(other, "A");
      EXPECT_EQ(inner.getKeys(), (std::set<std::string>{"A"}));
    }
    getNumber<double>(input, "C");
    EXPECT_EQ(recorder.getKeys(), (std::set<std::string>{"A", "B", "C"}));
  }

  // Nothing is recorded once the recorder is gone
  getNumber<int>(input, "A");
}
