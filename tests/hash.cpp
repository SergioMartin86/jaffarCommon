#include "gtest/gtest.h"
#include <jaffarCommon/hash.hpp>

using namespace jaffarCommon::hash;

TEST(hash, SHA1)
{
  EXPECT_EQ(getSHA1String(""), "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709");
  EXPECT_EQ(getSHA1String("abc"), "A9993E364706816ABA3E25717850C26C9CD0D89D");
  EXPECT_EQ(getSHA1String("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"), "A49B2446A02C645BF419F995B67091253A04A259");
}

TEST(hash, calculateMetroHash)
{
  std::string inputString = "012345678901234567890123456789012345678901234567890123456789012";

  hash_t value1;
  EXPECT_NO_THROW(value1 = calculateMetroHash(inputString.data(), inputString.size()));
  EXPECT_EQ(value1.first,  0x9B9FEDA4BFE27CC7);
  EXPECT_EQ(value1.second, 0x97A27450ACB24805);

  hash_t value2;
  EXPECT_NO_THROW(value2 = hashString(inputString));
  EXPECT_EQ(value2.first,  0x9B9FEDA4BFE27CC7);
  EXPECT_EQ(value2.second, 0x97A27450ACB24805);
}

TEST(hash, hashToString)
{
  hash_t value;
  value.first = 0x0011223344556677;
  value.second = 0x8899AABBCCDDEEFF;
  EXPECT_EQ(hashToString(value), "0x00112233445566778899AABBCCDDEEFF");
}