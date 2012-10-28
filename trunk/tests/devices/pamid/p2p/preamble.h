#include <hwi/include/bqc/A2_inlines.h>

#define PRINT_SUCCESS 0

//#define SLEEP sleep
#define SLEEP usleep

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

void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

