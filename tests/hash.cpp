#include "gtest/gtest.h"
#include "../include/hash.hpp"

using namespace jaffarCommon::hash;

TEST(hash, SHA1)
{
  EXPECT_EQ(getSHA1String(""), "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709");
}

