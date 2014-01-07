#ifndef PREAMBLE_H
#define PREAMBLE_H

#include <pami.h>

#define PRINT_SUCCESS 1

#define TEST_ASSERT(c,m) \
        do { \
        if (!(c)) { \
                    printf(m" FAILED on rank %ld\n", world_rank); \
                    fflush(stdout); \
                  } \
        else if (PRINT_SUCCESS) { \
                    printf(m" SUCCEEDED on rank %ld\n", world_rank); \
                    fflush(stdout); \
                  } \
        assert(c); \
        } \
        while(0);

static size_t world_size, world_rank = -1;

static void cb_done(pami_context_t ctxt, void * cookie, pami_result_t result)
{
  int * ptr = (int *) cookie;
  (*ptr)--;
  return;
}

#ifndef NOT_TEST_MAIN
pami_context_t * contexts;

pthread_t Progress_thread;

static void * Progress_function(void * dummy)
{
    pami_result_t result = PAMI_ERROR;

    while (1)
    {
        result = PAMI_Context_trylock_advancev(&(contexts[1]), 1, 1000);
        TEST_ASSERT(result == PAMI_SUCCESS || result == PAMI_EAGAIN,"PAMI_Context_trylock_advancev");
        usleep(1);
    }

    return NULL;
}
#endif

#endif
