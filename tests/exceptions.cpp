#include "gtest/gtest.h"
#include "../include/exceptions.hpp"

TEST(exceptions, runtime)
{
  ASSERT_THROW(JAFFAR_THROW_RUNTIME("Test"), std::runtime_error);
}

TEST(exceptions, logic)
{
  ASSERT_THROW(JAFFAR_THROW_LOGIC("Test"), std::logic_error);
}

TEST(exceptions, badCall)
{
  ASSERT_THROW(jaffarCommon::exceptions::throwException("", "", 1, ""), std::invalid_argument);
}