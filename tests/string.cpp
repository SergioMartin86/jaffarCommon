#include "gtest/gtest.h"
#include <jaffarCommon/string.hpp>

using namespace jaffarCommon::string;

TEST(string, split)
{
  std::string input("a,b,c");
  std::vector<std::string> expected = { "a", "b", "c" };
  auto output = split(input, ','); 
  EXPECT_EQ(output, expected);
}

TEST(string, formatString)
{
  std::string expected = "hello1";
  auto output = formatString("%s%1d", "hello", 1);
  EXPECT_EQ(output, expected);
}