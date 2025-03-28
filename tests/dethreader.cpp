#include <fstream>
#include <cstdio> 
#include "gtest/gtest.h"
#include <jaffarCommon/dethreader.hpp>

__JAFFAR_COMMON_DETHREADER_STATE

using namespace jaffarCommon::dethreader;

TEST(dethreader, simpleTest)
{
  // Instantiating runtime
  auto r = new Runtime();
  r->createThread([]() 
  {
     printf("[Thread 1] Started\n");
     printf("[Thread 1] Suspending...\n");
     Runtime::yield();
     printf("[Thread 1] Continuing...\n");
     printf("[Thread 1] Finished\n");
  });

  r->createThread([]() 
  {
     printf("[Thread 2] Started\n");
     printf("[Thread 2] Suspending...\n");
     Runtime::yield();
     printf("[Thread 2] Continuing...\n");
     printf("[Thread 2] Finished\n");
  });

  r->run();
}

TEST(dethreader, sleepTest)
{
  // Instantiating runtime
  auto r = new Runtime();
  r->createThread([]() 
  {
     printf("[Thread 1] Started\n");
     printf("[Thread 1] Sleeping...\n");
     Runtime::sleep(1 * 1000000); // 1 Second
     printf("[Thread 1] Continuing...\n");
     printf("[Thread 1] Finished\n");
  });

  r->createThread([]() 
  {
     printf("[Thread 2] Started\n");
     printf("[Thread 2] Finished\n");
  });

  r->run();
}
