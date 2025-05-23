/*
  note this was designed for UNIX systems. Based on ideas expressed in a paper by Ralf Engelschall.
  for SJLJ on other systems, one would want to rewrite springboard() and co_create() and hack the jmb_buf stack pointer.
*/

#define LIBCO_C
#include "libco.h"
#include "settings.h"

#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  sigjmp_buf context;
  void (*coentry)(void);
  void* stack;
} cothread_struct;

static thread_local cothread_struct co_primary;
static thread_local cothread_struct* creating;
static thread_local cothread_struct* co_running = 0;

static void springboard(int ignored) {
  if(sigsetjmp(creating->context, 0)) {
    co_running->coentry();
  }
}

inline cothread_t co_active(void) {
  if(!co_running) co_running = &co_primary;
  return (cothread_t)co_running;
}

cothread_t co_derive(void* memory, unsigned int size, void (*coentry)(void)) {
  if(!co_running) co_running = &co_primary;

  cothread_struct* thread = (cothread_struct*)memory;
  memory = (unsigned char*)memory + sizeof(cothread_struct);
  size -= sizeof(cothread_struct);
  if(thread) {
    struct sigaction handler;
    struct sigaction old_handler;

    stack_t stack;
    stack_t old_stack;

    thread->coentry = thread->stack = 0;

    stack.ss_flags = 0;
    stack.ss_size = size;
    thread->stack = stack.ss_sp = memory;
    if(stack.ss_sp && !sigaltstack(&stack, &old_stack)) {
      handler.sa_handler = springboard;
      handler.sa_flags = SA_ONSTACK;
      sigemptyset(&handler.sa_mask);
      creating = thread;

      if(!sigaction(SIGUSR1, &handler, &old_handler)) {
        if(!raise(SIGUSR1)) {
          thread->coentry = coentry;
        }
        sigaltstack(&old_stack, 0);
        sigaction(SIGUSR1, &old_handler, 0);
      }
    }

    if(thread->coentry != coentry) {
      co_delete(thread);
      thread = 0;
    }
  }

  return (cothread_t)thread;
}

cothread_t co_create(unsigned int size, void (*coentry)(void)) {
  if(!co_running) co_running = &co_primary;

  cothread_struct* thread = (cothread_struct*)malloc(sizeof(cothread_struct));
  if(thread) {
    struct sigaction handler;
    struct sigaction old_handler;

    stack_t stack;
    stack_t old_stack;

    thread->coentry = thread->stack = 0;

    stack.ss_flags = 0;
    stack.ss_size = size;
    thread->stack = stack.ss_sp = malloc(size);
    if(stack.ss_sp && !sigaltstack(&stack, &old_stack)) {
      handler.sa_handler = springboard;
      handler.sa_flags = SA_ONSTACK;
      sigemptyset(&handler.sa_mask);
      creating = thread;

      if(!sigaction(SIGUSR1, &handler, &old_handler)) {
        if(!raise(SIGUSR1)) {
          thread->coentry = coentry;
        }
        sigaltstack(&old_stack, 0);
        sigaction(SIGUSR1, &old_handler, 0);
      }
    }

    if(thread->coentry != coentry) {
      co_delete(thread);
      thread = 0;
    }
  }

  return (cothread_t)thread;
}

void co_delete(cothread_t cothread) {
  if(cothread) {
    if(((cothread_struct*)cothread)->stack) {
      free(((cothread_struct*)cothread)->stack);
    }
    free(cothread);
  }
}

void co_switch(cothread_t cothread) {
  if(!sigsetjmp(co_running->context, 0)) {
    co_running = (cothread_struct*)cothread;
    siglongjmp(co_running->context, 1);
  }
}

int co_serializable(void) {
  return 0;
}

#ifdef __cplusplus
}
#endif
