#include "gtest/gtest.h"
#include "../include/timing.hpp"

using namespace jaffarCommon::timing;

TEST(timing, tests)
{
  auto t0 = now();
  auto t1 = now();
  auto diffs1 = timeDeltaSeconds(t1, t0);
  auto diffns1 = timeDeltaNanoseconds(t1, t0);
  auto t2 = now();
  auto diffs2 = timeDeltaSeconds(t2, t0);
  auto diffns2 = timeDeltaNanoseconds(t2, t0);
  EXPECT_GE(diffs2, diffs1);
  EXPECT_GE(diffns2, diffns1);
}
