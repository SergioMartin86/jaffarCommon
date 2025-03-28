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
     const auto threadId = dethreader::getCurrentThread()->getThreadId();
     printf("[Thread %lu] Started\n", threadId);
     printf("[Thread %lu] Suspending...\n", threadId);
     dethreader::yield();
     printf("[Thread %lu] Continuing...\n", threadId);
     printf("[Thread %lu] Finished\n", threadId);
  });

  dethreader::createThread([]() 
  {
     const auto threadId = dethreader::getCurrentThread()->getThreadId();
     printf("[Thread %lu] Started\n", threadId);
     printf("[Thread %lu] Suspending...\n", threadId);
     dethreader::yield();
     printf("[Thread %lu] Continuing...\n", threadId);
     printf("[Thread %lu] Finished\n", threadId);
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
     const auto threadId = dethreader::getCurrentThread()->getThreadId();
     printf("[Thread %lu] Started\n", threadId);
     printf("[Thread %lu] Sleeping...\n", threadId);
     dethreader::sleep(1 * 1000000); // 1 Second
     printf("[Thread %lu] Continuing...\n", threadId);
     printf("[Thread %lu] Finished\n", threadId);
  });

  dethreader::createThread([]() 
  {
     const auto threadId = dethreader::getCurrentThread()->getThreadId();
     printf("[Thread %lu] Started\n", threadId);
     printf("[Thread %lu] Finished\n", threadId);
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
     const auto threadId = dethreader::getCurrentThread()->getThreadId();
     printf("[Thread %lu] Started\n", threadId);
     printf("[Thread %lu] Sleeping...\n", threadId);

     const auto newThreadId = dethreader::createThread([]() 
     {
        const auto threadId = dethreader::getCurrentThread()->getThreadId();
        printf("[Thread %lu] Started\n", threadId);
        dethreader::sleep(1 * 1000000); // 1 Second
        printf("[Thread %lu] Finished\n", threadId);
     });

     printf("[Thread %lu] Waiting for thread %lu...\n", threadId, newThreadId);
     dethreader::join(newThreadId);

     printf("[Thread %lu] Continuing...\n", threadId);
     printf("[Thread %lu] Finished\n", threadId);
  });

  r->run();
  r->finalize();
}
