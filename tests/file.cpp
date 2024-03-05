#include <fstream>
#include <cstdio> 
#include "gtest/gtest.h"
#include <jaffarCommon/file.hpp>

using namespace jaffarCommon::file;

TEST(file, loadFile)
{
  std::string fileName = "testFile1.txt";
  std::string expectedContents = "Hello World!";

  std::string fileContents = "";
  ASSERT_FALSE(loadStringFromFile(fileContents, "WrongPath.txt"));
  ASSERT_TRUE(loadStringFromFile(fileContents, fileName));
  ASSERT_EQ(fileContents, expectedContents);
}


TEST(file, saveFile)
{
  std::string fileName = "testFile2.txt";
  std::string expectedContents = "Hello World!";
  ASSERT_FALSE(saveStringToFile(expectedContents, "/dev/null/foo"));
  ASSERT_TRUE(saveStringToFile(expectedContents, fileName));

  std::string fileContents;
  std::ifstream t;
  t.open(fileName);
  std::getline(t, fileContents);
  t.close();

  ASSERT_EQ(expectedContents, fileContents);
}