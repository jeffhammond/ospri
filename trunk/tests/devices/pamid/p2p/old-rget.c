#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

#include "safemalloc.h"
#include "preamble.h"
#include "coll.h"

int main(int argc, char* argv[])
{
  pami_result_t        result        = PAMI_ERROR;

  /* initialize the client */
  char * clientname = "";
  pami_client_t client;
  result = PAMI_Client_create(clientname, &client, NULL, 0);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_create");

  /* query properties of the client */
  pami_configuration_t config;
  size_t num_contexts;

  config.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query( client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_size = config.value.intval;

  config.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query( client, &config,1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_rank = config.value.intval;
  printf("hello world from rank %ld of %ld \n", world_rank, world_size );
  fflush(stdout);
  SLEEP(1);

  config.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query( client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  num_contexts = config.value.intval;

  /* initialize the contexts */
  pami_context_t * contexts;
  contexts = (pami_context_t *) malloc( num_contexts * sizeof(pami_context_t) );
  TEST_ASSERT(contexts!=NULL,"malloc");

  result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  printf("%ld contexts were created by rank %ld \n", num_contexts, world_rank );
  fflush(stdout);
  SLEEP(1);

  /* setup the world geometry */
  pami_geometry_t world_geometry;

  result = PAMI_Geometry_world( client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  pami_xfer_type_t barrier_xfer   = PAMI_XFER_BARRIER;
  size_t num_barrier_alg[2];

  result = PAMI_Geometry_algorithms_num( world_geometry, barrier_xfer, num_barrier_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num - barrier");

  pami_algorithm_t * safe_barrier_algs = (pami_algorithm_t *) malloc( num_barrier_alg[0] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * safe_barrier_meta = (pami_metadata_t  *) malloc( num_barrier_alg[0] * sizeof(pami_metadata_t)  );
  pami_algorithm_t * fast_barrier_algs = (pami_algorithm_t *) malloc( num_barrier_alg[1] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * fast_barrier_meta = (pami_metadata_t  *) malloc( num_barrier_alg[1] * sizeof(pami_metadata_t)  );
  TEST_ASSERT(safe_barrier_algs!=NULL && safe_barrier_meta!=NULL && fast_barrier_algs!=NULL && fast_barrier_meta!=NULL,"malloc");
  result = PAMI_Geometry_algorithms_query( world_geometry, barrier_xfer,
                                           safe_barrier_algs, safe_barrier_meta, num_barrier_alg[0],
                                           fast_barrier_algs, fast_barrier_meta, num_barrier_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query - barrier");

  pami_xfer_type_t allgather_xfer = PAMI_XFER_ALLGATHER;
  size_t num_allgather_alg[2];

  result = PAMI_Geometry_algorithms_num( world_geometry, allgather_xfer, num_allgather_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num - allgather");

  pami_algorithm_t * safe_allgather_algs = (pami_algorithm_t *) malloc( num_allgather_alg[0] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * safe_allgather_meta = (pami_metadata_t  *) malloc( num_allgather_alg[0] * sizeof(pami_metadata_t)  );
  pami_algorithm_t * fast_allgather_algs = (pami_algorithm_t *) malloc( num_allgather_alg[1] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * fast_allgather_meta = (pami_metadata_t  *) malloc( num_allgather_alg[1] * sizeof(pami_metadata_t)  );
  TEST_ASSERT(safe_allgather_algs!=NULL && safe_allgather_meta!=NULL && fast_allgather_algs!=NULL && fast_allgather_meta!=NULL,"malloc");
  result = PAMI_Geometry_algorithms_query( world_geometry, allgather_xfer,
                                           safe_allgather_algs, safe_allgather_meta, num_allgather_alg[0],
                                           fast_allgather_algs, fast_allgather_meta, num_allgather_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query - allgather");

  /* initialize the allgather buffers */
  size_t alg     = ( argc > 1 ? atoi(argv[1]) : 0 );
  size_t bufsize = ( argc > 2 ? atoi(argv[2]) : 1000 );

  int * sbuf = NULL;
  int * rbuf = NULL;

  sbuf = malloc( bufsize * sizeof(int) );
  rbuf = malloc( bufsize * world_size * sizeof(int) );
  TEST_ASSERT(sbuf!=NULL && rbuf!=NULL,"malloc");

  size_t i, j;
  for ( i = 0 ; i < bufsize ; i++ )                  sbuf[i] = (int) world_rank;
  for ( i = 0 ; i < ( bufsize * world_size ) ; i++ ) rbuf[i] = -1;

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

  /* perform allgather */
  pami_xfer_t allgather;

  allgather.cb_done   = cb_done;
  allgather.cookie    = (void*) &active;
  allgather.algorithm = safe_allgather_algs[0];

  allgather.cmd.xfer_allgather.sndbuf     = (void*) sbuf;
  allgather.cmd.xfer_allgather.stype      = PAMI_TYPE_SIGNED_INT;
  allgather.cmd.xfer_allgather.stypecount = bufsize;
  allgather.cmd.xfer_allgather.rcvbuf     = (void*) rbuf;
  allgather.cmd.xfer_allgather.rtype      = PAMI_TYPE_SIGNED_INT;
  allgather.cmd.xfer_allgather.rtypecount = bufsize;

  active = 1;
  if ( world_rank == 0 ) printf("trying allgather of %ld bytes with algorithm %ld (%s) \n", bufsize, alg, safe_allgather_meta[alg].name );
  result = PAMI_Collective( contexts[0], &allgather );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - allgather");
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

  for ( i = 0 ; i < world_size ; i++ )
    for ( j = 0 ; j < bufsize ; j++ ) 
      if ( rbuf[ i * bufsize + j ] != i)
        printf("%ld: rbuf[%ld] = %d \n", world_rank, i * bufsize + j, rbuf[ i * bufsize + j ] );
  if ( world_rank == 0 ) printf("allgather successful\n");

  /* finalize the contexts */
  result = PAMI_Context_destroyv( contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_destroyv");

  free(contexts);

  /* finalize the client */
  result = PAMI_Client_destroy( &client );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_destroy");

  printf("%ld: end of test \n", world_rank );
  fflush(stdout);
  SLEEP(1);

  return 0;
}

