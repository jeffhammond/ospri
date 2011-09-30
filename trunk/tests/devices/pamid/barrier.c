#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

static size_t world_size, world_rank = -1;

#define PRINT_SUCCESS 0

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
        sleep(1); \
        /* assert(c);*/ \
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
  size_t num_contexts;

  config.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query( &client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_rank = config.value.intval;

  config.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query( &client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_size = config.value.intval;

  if ( world_rank == 0 ) 
  {
    printf("starting test on %ld ranks \n", world_size);
    fflush(stdout);
  }
  sleep(1);

  config.name = PAMI_CLIENT_PROCESSOR_NAME;
  result = PAMI_Client_query(&client, &config,1);
  assert(result == PAMI_SUCCESS);
  printf("rank %ld is processor %s \n", world_rank, config.value.chararray);
  fflush(stdout);
  sleep(1);

  config.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query( &client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  num_contexts = config.value.intval;

  /* initialize the contexts */
  pami_context_t * contexts = NULL;
  contexts = (pami_context_t *) malloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  /* setup the world geometry */
  pami_geometry_t world_geometry;
  pami_xfer_type_t barrier_xfer = PAMI_XFER_BARRIER;
  size_t num_alg[2];
  pami_algorithm_t * safe_barrier_algs = NULL;
  pami_metadata_t  * safe_barrier_meta = NULL;
  pami_algorithm_t * fast_barrier_algs = NULL;
  pami_metadata_t  * fast_barrier_meta = NULL;

  result = PAMI_Geometry_world( client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  result = PAMI_Geometry_algorithms_num( world_geometry, barrier_xfer, num_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");
  if ( world_rank == 0 ) printf("number of barrier algorithms = {%ld,%ld} \n", num_alg[0], num_alg[1] );

  safe_barrier_algs = (pami_algorithm_t *) malloc( num_alg[0] * sizeof(pami_algorithm_t) );
  safe_barrier_meta = (pami_metadata_t  *) malloc( num_alg[0] * sizeof(pami_metadata_t)  );
  fast_barrier_algs = (pami_algorithm_t *) malloc( num_alg[1] * sizeof(pami_algorithm_t) );
  fast_barrier_meta = (pami_metadata_t  *) malloc( num_alg[1] * sizeof(pami_metadata_t)  );
  result = PAMI_Geometry_algorithms_query( world_geometry, barrier_xfer,
                                           safe_barrier_algs, safe_barrier_meta, num_alg[0],
                                           fast_barrier_algs, fast_barrier_meta, num_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

  /* perform a barrier */
  size_t b;
  pami_xfer_t barrier;
  volatile int active = 0;

  for ( b = 0 ; b < num_alg[0] ; b++ )
    /* disable broken algorithms */
    if ( b != 2 && b != 3 && b != 5 )
    {
      barrier.cb_done   = cb_done;
      barrier.cookie    = (void*) &active;
      barrier.algorithm = safe_barrier_algs[b];

      if ( world_rank == 0 ) printf("trying barrier algorithm %ld (%s) \n", b, safe_barrier_meta[b].name );
      fflush(stdout);
      sleep(1);

      active = 1;
      result = PAMI_Collective( contexts[0], &barrier );
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - barrier");
      while (active)
        result = PAMI_Context_advance( contexts[0], 1 );
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance - barrier");

      if ( world_rank == 0 ) printf("after barrier algorithm %ld (%s) \n", b, safe_barrier_meta[b].name );
      fflush(stdout);
      sleep(1);
    }

  /* finalize the contexts */
  result = PAMI_Context_destroyv( contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_destroyv");

  free(contexts);

  /* finalize the client */
  result = PAMI_Client_destroy( &client );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_destroy");

  if ( world_rank == 0 ) 
  {
    printf("end of test \n");
    fflush(stdout);
  }
  sleep(1);

  return 0;
}

