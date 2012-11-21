#ifndef PREAMBLE_H
#define PREAMBLE_H

#include <pami.h>

#define PRINT_SUCCESS 0

#ifdef DEBUG
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
        SLEEP(1); \
        assert(c); \
        } \
        while(0);
#else
#define TEST_ASSERT(c,m) 
#endif

static size_t world_size, world_rank = -1;

static void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

pami_context_t * contexts;

#ifdef PROGRESS_THREAD
pthread_t Progress_thread;

static void * Progress_function(void * dummy)
{
	pami_result_t result = PAMI_ERROR;

	while (1)
	{
        result = PAMI_Context_trylock_advancev(&(contexts[1]), 1, 1000);
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
		usleep(1);
	}

	return NULL;
}
#else
#warning No progress enabled - may not work.
#endif

#endif