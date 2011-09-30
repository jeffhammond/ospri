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
  size_t num_contexts;

  config.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query( &client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_size = config.value.intval;

  config.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query( &client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_rank = config.value.intval;
  printf("hello world from rank %ld of %ld \n", world_rank, world_size );
  fflush(stdout);
  sleep(1);

  config.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query( &client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  num_contexts = config.value.intval;

  /* initialize the contexts */
  pami_context_t * contexts;
  contexts = (pami_context_t *) malloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  printf("%ld contexts were created by rank %ld \n", num_contexts, world_rank );
  fflush(stdout);
  sleep(1);

  /* setup the world geometry */
  pami_geometry_t world_geometry;

  result = PAMI_Geometry_world( client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  pami_xfer_type_t barrier_xfer   = PAMI_XFER_BARRIER;
  size_t num_barrier_alg[2];
  pami_algorithm_t * safe_barrier_algs = NULL;
  pami_metadata_t  * safe_barrier_meta = NULL;
  pami_algorithm_t * fast_barrier_algs = NULL;
  pami_metadata_t  * fast_barrier_meta = NULL;

  result = PAMI_Geometry_algorithms_num( world_geometry, barrier_xfer, num_barrier_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num - barrier");

  safe_barrier_algs = (pami_algorithm_t *) malloc( num_barrier_alg[0] * sizeof(pami_algorithm_t) );
  safe_barrier_meta = (pami_metadata_t  *) malloc( num_barrier_alg[0] * sizeof(pami_metadata_t)  );
  fast_barrier_algs = (pami_algorithm_t *) malloc( num_barrier_alg[1] * sizeof(pami_algorithm_t) );
  fast_barrier_meta = (pami_metadata_t  *) malloc( num_barrier_alg[1] * sizeof(pami_metadata_t)  );
  result = PAMI_Geometry_algorithms_query( world_geometry, barrier_xfer,
                                           safe_barrier_algs, safe_barrier_meta, num_barrier_alg[0],
                                           fast_barrier_algs, fast_barrier_meta, num_barrier_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query - barrier");

  pami_xfer_type_t allreduce_xfer = PAMI_XFER_ALLREDUCE;
  size_t num_allreduce_alg[2];
  pami_algorithm_t * safe_allreduce_algs = NULL;
  pami_metadata_t  * safe_allreduce_meta = NULL;
  pami_algorithm_t * fast_allreduce_algs = NULL;
  pami_metadata_t  * fast_allreduce_meta = NULL;

  result = PAMI_Geometry_algorithms_num( world_geometry, allreduce_xfer, num_allreduce_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num - allreduce");
  if ( world_rank == 0 ) printf("number of allreduce algorithms = {%ld,%ld} \n", num_allreduce_alg[0], num_allreduce_alg[1] );

  safe_allreduce_algs = (pami_algorithm_t *) malloc( num_allreduce_alg[0] * sizeof(pami_algorithm_t) );
  safe_allreduce_meta = (pami_metadata_t  *) malloc( num_allreduce_alg[0] * sizeof(pami_metadata_t)  );
  fast_allreduce_algs = (pami_algorithm_t *) malloc( num_allreduce_alg[1] * sizeof(pami_algorithm_t) );
  fast_allreduce_meta = (pami_metadata_t  *) malloc( num_allreduce_alg[1] * sizeof(pami_metadata_t)  );
  result = PAMI_Geometry_algorithms_query( world_geometry, allreduce_xfer,
                                           safe_allreduce_algs, safe_allreduce_meta, num_allreduce_alg[0],
                                           fast_allreduce_algs, fast_allreduce_meta, num_allreduce_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query - allreduce");

  /* initialize the allreduce buffers */
  size_t alg     = ( argc > 1 ? atoi(argv[1]) : 0 );
  size_t bufsize = ( argc > 2 ? atoi(argv[2]) : 1000 );

  int * sbuf = NULL;
  int * rbuf = NULL;

  sbuf = malloc( bufsize * sizeof(int) );
  rbuf = malloc( bufsize * sizeof(int) );

  size_t i, j;
  for ( i = 0 ; i < bufsize ; i++ ) sbuf[i] = (int) world_rank;
  for ( i = 0 ; i < bufsize ; i++ ) rbuf[i] = -1;

  /* perform a barrier */
  pami_xfer_t barrier;
  volatile int active = 0;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) &active;
  barrier.algorithm = safe_barrier_algs[0];

  active = 1;
  result = PAMI_Collective( contexts[0], &barrier );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - barrier");
  while (active)
    result = PAMI_Context_advance( contexts[0], 1 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");

  /* perform allreduce */
  pami_xfer_t allreduce;

  allreduce.cb_done   = cb_done;
  allreduce.cookie    = (void*) &active;
  allreduce.algorithm = safe_allreduce_algs[0];

  allreduce.cmd.xfer_allreduce.sndbuf     = (void*) sbuf;
  allreduce.cmd.xfer_allreduce.stype      = PAMI_TYPE_SIGNED_INT;
  allreduce.cmd.xfer_allreduce.stypecount = bufsize;
  allreduce.cmd.xfer_allreduce.rcvbuf     = (void*) rbuf;
  allreduce.cmd.xfer_allreduce.rtype      = PAMI_TYPE_SIGNED_INT;
  allreduce.cmd.xfer_allreduce.rtypecount = bufsize;
  allreduce.cmd.xfer_allreduce.op         = PAMI_DATA_SUM;

  active = 1;
  if ( world_rank == 0 ) printf("trying allreduce of %ld bytes with algorithm %ld (%s) \n", bufsize, alg, safe_allreduce_meta[alg].name );
  result = PAMI_Collective( contexts[0], &allreduce );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - allreduce");
  while (active)
    result = PAMI_Context_advance( contexts[0], 1 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");

  /* perform a barrier */
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) &active;
  barrier.algorithm = safe_barrier_algs[0];

  active = 1;
  result = PAMI_Collective( contexts[0], &barrier );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - barrier");
  while (active)
    result = PAMI_Context_advance( contexts[0], 1 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");

  int correct = world_size/2 * ( world_size - 1 ); // world_size should be a multiple of 2
  for ( j = 0 ; j < bufsize ; j++ ) 
    if ( rbuf[ j ] != correct )
        printf("%ld: rbuf[%ld] = %d \n", world_rank, i * bufsize + j, rbuf[ i * bufsize + j ] );
  if ( world_rank == 0 ) printf("allreduce successful\n");

  /* finalize the contexts */
  result = PAMI_Context_destroyv( contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_destroyv");

  free(contexts);

  /* finalize the client */
  result = PAMI_Client_destroy( &client );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_destroy");

  printf("%ld: end of test \n", world_rank );
  fflush(stdout);
  sleep(1);

  return 0;
}

