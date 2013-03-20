#ifndef PREAMBLE_H
#define PREAMBLE_H

#include <pami.h>

#define PRINT_SUCCESS 0

#ifdef DEBUG
#define TEST_ASSERT(c,m) \
        do { \
        if (!(c)) { \
                    printf(m" FAILED on rank %ld\n", g_world_rank); \
                    fflush(stdout); \
                    sleep(1); \
                    abort(); \
                  } \
        else if (PRINT_SUCCESS) { \
                    printf(m" SUCCEEDED on rank %ld\n", g_world_rank); \
                    fflush(stdout); \
                  } \
        } \
        while(0);
#else
#define TEST_ASSERT(c,m) 
#endif

static size_t g_world_rank = -1;

static void cb_done (void * ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

typedef struct done_s
{
    int local;
    int remote;
} 
done_t;

static void cb_done_local(void * ctxt, void * clientdata, pami_result_t err)
{
    done_t * temp = (done_t *) clientdata;
    (temp->local)--;
}

static void cb_done_remote(void * ctxt, void * clientdata, pami_result_t err)
{
    done_t * temp = (done_t *) clientdata;
    (temp->remote)--;
}

pami_context_t * contexts;

#ifdef PROGRESS_THREAD
pthread_t Progress_thread;

static void * Progress_function(void * input)
{
	pami_result_t result = PAMI_ERROR;

    int * ptr        = (int *) input;
    int   my_context = (ptr!=NULL) ? (*ptr) : 1;

#ifdef DEBUG
    printf("%ld: Progress_function advancing context %d \n", g_world_rank, my_context);
#endif

	while (1)
	{
        result = PAMI_Context_trylock_advancev(&(contexts[my_context]), 1, 1000);
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
		usleep(1);
	}

	return NULL;
}
#else
#warning No progress enabled - may not work.
#endif

#endif
