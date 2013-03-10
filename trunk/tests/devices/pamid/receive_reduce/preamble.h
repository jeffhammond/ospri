#ifndef PREAMBLE_H
#define PREAMBLE_H

#include <pami.h>

static size_t world_size, world_rank = -1;

#define PRINT_SUCCESS 0

#ifdef DEBUG
#define TEST_ASSERT(c,m) \
        do { \
        if (!(c)) { \
                    printf(m" FAILED on rank %ld\n", world_rank); \
                    fflush(stdout); \
                    sleep(1); \
                    abort(); \
                  } \
        else if (PRINT_SUCCESS) { \
                    printf(m" SUCCEEDED on rank %ld\n", world_rank); \
                    fflush(stdout); \
                  } \
        } \
        while(0);
#else
#define TEST_ASSERT(c,m) 
#endif

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

#endif
