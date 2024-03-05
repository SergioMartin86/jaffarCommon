#include "gtest/gtest.h"
#include <jaffarCommon/loggers/stdio.hpp>

using namespace jaffarCommon::logger;

TEST(logger, terminal)
{
  //waitForKeyPress() { return getchar(); }
  EXPECT_EQ(getKeyPress(),0);
  EXPECT_NO_THROW(initializeTerminal());
  EXPECT_NO_THROW(clearTerminal());
  EXPECT_NO_THROW(log("Test"));
  EXPECT_NO_THROW(refreshTerminal());
  EXPECT_NO_THROW(finalizeTerminal());
}
