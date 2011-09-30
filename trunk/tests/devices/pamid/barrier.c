#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

static int world_size, world_rank = -1;

#define TEST_ASSERT(c,m) \
        do { \
        if (!(c)) { \
                    printf(m" FAILED on rank %d\n", world_rank); \
                    fflush(stdout); \
                  } \
        else { \
                    printf(m" SUCCEEDED on rank %d\n", world_rank); \
                    fflush(stdout); \
                  } \
        /*assert(c);*/ \
        } \
        while(0);

void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

int main(int argc, char* argv[])
{
  pami_result_t        result        = PAMI_ERROR;

  /* initialize the client */
  char * clientname = "JEFF";
  pami_client_t client;
  result = PAMI_Client_create(clientname, &client, NULL, 0);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_create");

  /* query properties of the client */
  pami_configuration_t config;
  int num_contexts;

  config.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query( &client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_size = config.value.intval;

  config.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query( &client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_rank = config.value.intval;
  printf("hello world from rank %d of %d \n",world_rank,world_size);
  fflush(stdout);
  sleep(1);

  config.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query( &client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  num_contexts = config.value.intval;

  /* initialize the contexts */
  pami_context_t * contexts;
  contexts = (pami_context_t *) malloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  printf("%d contexts were created by rank %d \n",num_contexts,world_rank);
  fflush(stdout);
  sleep(1);

  /* setup the world geometry */
  pami_geometry_t world_geometry;
  pami_xfer_type_t barrier_xfer = PAMI_XFER_BARRIER;
  size_t num_alg[2];
  pami_algorithm_t * safe_barrier_algs;
  pami_algorithm_t * fast_barrier_algs;

  result = PAMI_Geometry_world( client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  result = PAMI_Geometry_algorithms_num( world_geometry, barrier_xfer, num_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");

  safe_barrier_algs = (pami_algorithm_t *) malloc( num_alg[0] * sizeof(pami_algorithm_t) );
  fast_barrier_algs = (pami_algorithm_t *) malloc( num_alg[1] * sizeof(pami_algorithm_t) );
  result = PAMI_Geometry_algorithms_query( world_geometry, barrier_xfer,
                                           safe_barrier_algs, NULL, num_alg[0],
                                           fast_barrier_algs, NULL, num_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

  /* perform a barrier */
  pami_xfer_t barrier;
  volatile int active = 0;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) &active;
  barrier.algorithm = safe_barrier_algs[0];

  result = PAMI_Collective( contexts[0], &barrier );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective(barrier)");
  while (active)
    result = PAMI_Context_advance( contexts[0], 1 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");

  /* finalize the contexts */
  result = PAMI_Context_destroyv( contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_destroyv");

  free(contexts);

  /* finalize the client */
  result = PAMI_Client_destroy( &client );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_destroy");

  printf("end of test \n");
  fflush(stdout);
  sleep(1);

  return 0;
}

