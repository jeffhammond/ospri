#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

//#define SLEEP sleep
#define SLEEP usleep

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
        SLEEP(1); \
        /*assert(c);*/ \
        } \
        while(0);

static size_t world_size, world_rank = -1;

void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

pami_client_t client;
pami_geometry_t world_geometry;
pami_context_t * contexts;

void barrier(void)
{
  pami_result_t result = PAMI_ERROR;

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

  /* perform a barrier */
  pami_xfer_t barrier;
  volatile int active = 0;

  active = 1;
  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) &active;
  barrier.algorithm = safe_barrier_algs[0];

  result = PAMI_Collective( contexts[0], &barrier );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - barrier");

  while (active)
    result = PAMI_Context_advance( contexts[0], 1 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");

  free(safe_barrier_algs);
  free(safe_barrier_meta);
  free(fast_barrier_algs);
  free(fast_barrier_meta);

  return;
}

void allgather(int bytes, void * sbuf, void * rbuf)
{
  pami_result_t result = PAMI_ERROR;

  size_t alg = 0; /* be safe */

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

  /* perform allgather */
  pami_xfer_t allgather;

  volatile int active = 0;
  allgather.cb_done   = cb_done;
  allgather.cookie    = (void*) &active;
  allgather.algorithm = safe_allgather_algs[0];

  active = 1;
  allgather.cmd.xfer_allgather.sndbuf     = sbuf;
  allgather.cmd.xfer_allgather.stype      = PAMI_TYPE_BYTE;
  allgather.cmd.xfer_allgather.stypecount = (size_t)bytes;
  allgather.cmd.xfer_allgather.rcvbuf     = rbuf;
  allgather.cmd.xfer_allgather.rtype      = PAMI_TYPE_BYTE;
  allgather.cmd.xfer_allgather.rtypecount = (size_t)bytes;

  if ( world_rank == 0 ) 
    printf("trying allgather of %d bytes with algorithm %ld (%s) \n", bytes, alg, safe_allgather_meta[alg].name );

  result = PAMI_Collective( contexts[0], &allgather );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - allgather");

  while (active)
    result = PAMI_Context_advance( contexts[0], 1 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");

  free(safe_allgather_algs);
  free(safe_allgather_meta);
  free(fast_allgather_algs);
  free(fast_allgather_meta);

  return;
}

int main(int argc, char* argv[])
{
  pami_result_t result = PAMI_ERROR;

  /* initialize the client */
  char * clientname = "";
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
  contexts = (pami_context_t *) malloc( num_contexts * sizeof(pami_context_t) );
  TEST_ASSERT(contexts!=NULL,"malloc");

  result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  printf("%ld contexts were created by rank %ld \n", num_contexts, world_rank );
  fflush(stdout);
  SLEEP(1);

  /* setup the world geometry */
  result = PAMI_Geometry_world( client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  barrier();

  /****************************************************************/

  int n = 1000;
  char * in  = (char *) malloc(n);
  char * out = (char *) malloc(n);
  memset(in,  '\0', n);
  memset(out, '\a', n);

  allgather(n, in, out);

  free(out);
  free(in);

  /****************************************************************/

  barrier();

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

