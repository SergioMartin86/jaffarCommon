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

TEST(file, memFile)
{
  const size_t size = 16;
  uint8_t* mySrcBuffer = (uint8_t*) malloc(size);
  uint8_t* myDstBuffer = (uint8_t*) malloc(size);
  for (size_t i = 0; i < size; i++) mySrcBuffer[i] = (uint8_t)i;

  MemoryFile f(size);

  ASSERT_FALSE(f.isOpened());
  ASSERT_NO_THROW(f.setOpened());
  ASSERT_TRUE(f.isOpened());
  ASSERT_NO_THROW(f.unsetOpened());
  ASSERT_FALSE(f.isOpened());

  ASSERT_FALSE(f.isReadOnly());
  ASSERT_NO_THROW(f.setReadOnly());
  ASSERT_TRUE(f.isReadOnly());
  ASSERT_NO_THROW(f.unsetReadOnly());
  ASSERT_FALSE(f.isReadOnly());

  ASSERT_FALSE(f.isWriteOnly());
  ASSERT_NO_THROW(f.setWriteOnly());
  ASSERT_TRUE(f.isWriteOnly());
  ASSERT_NO_THROW(f.unsetWriteOnly());
  ASSERT_FALSE(f.isWriteOnly());

  ASSERT_NO_THROW(f.setOpened());
  ASSERT_FALSE(MemoryFile::feof(&f));
  ASSERT_EQ(MemoryFile::fwrite(mySrcBuffer, size, 1, &f), size);
  ASSERT_TRUE(MemoryFile::feof(&f));
  ASSERT_LE(MemoryFile::fread(nullptr, size, 1, &f), 0);
  ASSERT_NO_THROW(MemoryFile::rewind(&f));
  ASSERT_EQ(MemoryFile::ftell(&f), 0);
  ASSERT_EQ(MemoryFile::fread(myDstBuffer, size, 1, &f), size);
  for (size_t i = 0; i < size; i++) ASSERT_EQ(myDstBuffer[i], mySrcBuffer[i]);
  ASSERT_TRUE(MemoryFile::feof(&f));


  ASSERT_EQ(MemoryFile::fseek(&f, 0, SEEK_SET), 0);
  ASSERT_EQ(MemoryFile::ftell(&f), 0);
  ASSERT_FALSE(MemoryFile::feof(&f));
  ASSERT_EQ(MemoryFile::fseek(&f, 1, SEEK_SET), 0);
  ASSERT_EQ(MemoryFile::ftell(&f), 1);
  ASSERT_EQ(MemoryFile::fseek(&f, 0, SEEK_END), 0);
  ASSERT_TRUE(MemoryFile::feof(&f));
  ASSERT_EQ(MemoryFile::ftell(&f), size);
  ASSERT_EQ(MemoryFile::fseek(&f, -1, SEEK_END), 0);
  ASSERT_FALSE(MemoryFile::feof(&f));
  ASSERT_EQ(MemoryFile::ftell(&f), size - 1);
  ASSERT_LT(MemoryFile::fwrite(mySrcBuffer, size, 1, &f), 0);

  // Write Callback testing
  {
    ssize_t writtenBytesCheck = 0;
    MemoryFile* filePointerCheck = nullptr;
    ASSERT_NO_THROW(f.setWriteCallback([&writtenBytesCheck, &filePointerCheck](const ssize_t writtenBytes, MemoryFile* const filePointer) { writtenBytesCheck = writtenBytes; filePointerCheck = filePointer; } ));
    ASSERT_NO_THROW(MemoryFile::rewind(&f));
    ASSERT_EQ(MemoryFile::fwrite(mySrcBuffer, size, 1, &f), size);
    ASSERT_EQ(writtenBytesCheck, size);
    ASSERT_EQ(filePointerCheck, &f);

    ASSERT_NO_THROW(f.unsetWriteCallback());
    writtenBytesCheck = 0;
    filePointerCheck = nullptr;
    ASSERT_NO_THROW(MemoryFile::rewind(&f));
    ASSERT_EQ(MemoryFile::fwrite(mySrcBuffer, size, 1, &f), size);
    ASSERT_EQ(writtenBytesCheck, 0);
    ASSERT_EQ(filePointerCheck, nullptr);
  }

  // Read Callback testing
  {
    ssize_t readBytesCheck = 0;
    MemoryFile* filePointerCheck = nullptr;
    ASSERT_NO_THROW(f.setReadCallback([&readBytesCheck, &filePointerCheck](const ssize_t readBytes, MemoryFile* const filePointer) { readBytesCheck = readBytes; filePointerCheck = filePointer; } ));
    ASSERT_NO_THROW(MemoryFile::rewind(&f));
    ASSERT_EQ(MemoryFile::fread(mySrcBuffer, size, 1, &f), size);
    ASSERT_EQ(readBytesCheck, size);
    ASSERT_EQ(filePointerCheck, &f);
    
    ASSERT_NO_THROW(f.unsetReadCallback());
    readBytesCheck = 0;
    filePointerCheck = nullptr;
    ASSERT_NO_THROW(MemoryFile::rewind(&f));
    ASSERT_EQ(MemoryFile::fread(mySrcBuffer, size, 1, &f), size);
    ASSERT_EQ(readBytesCheck, 0);
    ASSERT_EQ(filePointerCheck, nullptr);
  }
}

TEST(file, memFileDirectory)
{
  MemoryFileDirectory d;

  size_t size = 16;
  std::string fileName = "file1";

  ASSERT_EQ(d.fopen(fileName, "r", size), (MemoryFile*)NULL);
  ASSERT_EQ(d.fopen(fileName, "r+", size), (MemoryFile*)NULL);
  ASSERT_EQ(d.fopen(fileName, "a", size), (MemoryFile*)NULL);
  ASSERT_EQ(d.fopen(fileName, "a+", size), (MemoryFile*)NULL);
  ASSERT_EQ(d.fopen(fileName, "", size), (MemoryFile*)NULL);
  ASSERT_EQ(d.fopen(fileName, "+", size), (MemoryFile*)NULL);

  MemoryFile* f = NULL;
  ASSERT_NE(d.fclose(f), 0);
  ASSERT_NE(f = d.fopen(fileName, "w", size), (MemoryFile*)NULL);
  ASSERT_TRUE(f->isOpened());
  ASSERT_TRUE(f->isWriteOnly());
  ASSERT_FALSE(f->isReadOnly());
  ASSERT_EQ(d.fopen(fileName, "w", size), (MemoryFile*)NULL);
  ASSERT_EQ(d.fclose(f), 0);
  ASSERT_NE(d.fclose(f), 0);
  ASSERT_FALSE(f->isOpened());

  f = NULL;
  ASSERT_NE(f = d.fopen(fileName, "r", size), (MemoryFile*)NULL);
  ASSERT_TRUE(f->isOpened());
  ASSERT_FALSE(f->isWriteOnly());
  ASSERT_TRUE(f->isReadOnly());
  ASSERT_EQ(d.fclose(f), 0);
  ASSERT_FALSE(f->isOpened());

  f = NULL;
  ASSERT_NE(f = d.fopen(fileName, "r+", size), (MemoryFile*)NULL);
  ASSERT_TRUE(f->isOpened());
  ASSERT_FALSE(f->isWriteOnly());
  ASSERT_FALSE(f->isReadOnly());
  ASSERT_EQ(d.fclose(f), 0);
  ASSERT_FALSE(f->isOpened());

  f = NULL;
  ASSERT_NE(f = d.fopen(fileName, "w+", size), (MemoryFile*)NULL);
  ASSERT_TRUE(f->isOpened());
  ASSERT_FALSE(f->isWriteOnly());
  ASSERT_FALSE(f->isReadOnly());
  ASSERT_EQ(d.fclose(f), 0);
  ASSERT_FALSE(f->isOpened());

  ASSERT_EQ(d.fdestroy(fileName), 0);
  ASSERT_NE(d.fdestroy(fileName), 0);
  ASSERT_EQ(d.fopen(fileName, "r", size), (MemoryFile*)NULL);
  ASSERT_NE(f = d.fopen(fileName, "w", size), (MemoryFile*)NULL);
}