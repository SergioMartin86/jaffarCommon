#include <fstream>
#include <cstdio> 
#include "gtest/gtest.h"
#include <jaffarCommon/dethreader.hpp>

__JAFFAR_COMMON_DETHREADER_STATE

using namespace jaffarCommon;

TEST(dethreader, simpleTest)
{
  // Instantiating runtime
  auto r = new dethreader::Runtime();
  r->initialize();

  dethreader::createThread([]() 
  {
     printf("[Thread 1] Started\n");
     printf("[Thread 1] Suspending...\n");
     dethreader::yield();
     printf("[Thread 1] Continuing...\n");
     printf("[Thread 1] Finished\n");
  });

  dethreader::createThread([]() 
  {
     printf("[Thread 2] Started\n");
     printf("[Thread 2] Suspending...\n");
     dethreader::yield();
     printf("[Thread 2] Continuing...\n");
     printf("[Thread 2] Finished\n");
  });

  r->run();
  r->finalize();
}

TEST(dethreader, sleepTest)
{
  // Instantiating runtime
  auto r = new dethreader::Runtime();
  r->initialize();

  dethreader::createThread([]() 
  {
     printf("[Thread 1] Started\n");
     printf("[Thread 1] Sleeping...\n");
     dethreader::sleep(1 * 1000000); // 1 Second
     printf("[Thread 1] Continuing...\n");
     printf("[Thread 1] Finished\n");
  });

  dethreader::createThread([]() 
  {
     printf("[Thread 2] Started\n");
     printf("[Thread 2] Finished\n");
  });

  r->run();
  r->finalize();
}

TEST(dethreader, join)
{
  // Instantiating runtime
  auto r = new dethreader::Runtime();
  r->initialize();

  dethreader::createThread([]() 
  {
     printf("[Thread 1] Started\n");
     printf("[Thread 1] Sleeping...\n");

     auto threadId = dethreader::createThread([]() 
     {
        printf("[Thread 2] Started\n");
        dethreader::sleep(1 * 1000000); // 1 Second
        printf("[Thread 2] Finished\n");
     });

     printf("[Thread 1] Waiting for thread %lu...\n", threadId);
     dethreader::join(threadId);

     printf("[Thread 1] Continuing...\n");
     printf("[Thread 1] Finished\n");
  });

  r->run();
  r->finalize();
}
