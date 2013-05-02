#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

#include "safemalloc.h"
#include "preamble.h"

int main(int argc, char* argv[])
{
  pami_result_t        result        = PAMI_ERROR;

  /* initialize the client */
  char * clientname = "";
  pami_client_t client;
  result = PAMI_Client_create( clientname, &client, NULL, 0 );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_create");

  /* query properties of the client */
  pami_configuration_t config;
  size_t num_contexts;

  config.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query( client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_rank = config.value.intval;

  config.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query( client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_size = config.value.intval;

  if ( world_rank == 0 )
  {
    printf("starting test on %ld ranks \n", world_size);
    fflush(stdout);
  }

  config.name = PAMI_CLIENT_PROCESSOR_NAME;
  result = PAMI_Client_query( client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  //printf("rank %ld is processor %s \n", world_rank, config.value.chararray);
  //fflush(stdout);

  config.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query( client, &config, 1);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  num_contexts = config.value.intval;

  /* initialize the contexts */
  pami_context_t * contexts = NULL;
  contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  /* setup the world geometry */
  pami_geometry_t world_geometry;

  result = PAMI_Geometry_world( client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  pami_xfer_type_t barrier_xfer = PAMI_XFER_BARRIER;
  size_t num_barrier_alg[2];

  /* barrier algs */
  result = PAMI_Geometry_algorithms_num( world_geometry, barrier_xfer, num_barrier_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");
  if ( world_rank == 0 ) printf("number of barrier algorithms = {%ld,%ld} \n", num_barrier_alg[0], num_barrier_alg[1] );

  pami_algorithm_t * safe_barrier_algs = (pami_algorithm_t *) safemalloc( num_barrier_alg[0] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * safe_barrier_meta = (pami_metadata_t  *) safemalloc( num_barrier_alg[0] * sizeof(pami_metadata_t)  );
  pami_algorithm_t * fast_barrier_algs = (pami_algorithm_t *) safemalloc( num_barrier_alg[1] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * fast_barrier_meta = (pami_metadata_t  *) safemalloc( num_barrier_alg[1] * sizeof(pami_metadata_t)  );
  result = PAMI_Geometry_algorithms_query( world_geometry, barrier_xfer,
                                           safe_barrier_algs, safe_barrier_meta, num_barrier_alg[0],
                                           fast_barrier_algs, fast_barrier_meta, num_barrier_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

  /* alltoall algs */
  pami_xfer_type_t alltoall_xfer   = PAMI_XFER_ALLTOALL;
  size_t num_alltoall_alg[2];

  result = PAMI_Geometry_algorithms_num( world_geometry, alltoall_xfer, num_alltoall_alg );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");
  if ( world_rank == 0 ) printf("number of alltoall algorithms = {%ld,%ld} \n", num_alltoall_alg[0], num_alltoall_alg[1] );

  pami_algorithm_t * safe_alltoall_algs = (pami_algorithm_t *) safemalloc( num_alltoall_alg[0] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * safe_alltoall_meta = (pami_metadata_t  *) safemalloc( num_alltoall_alg[0] * sizeof(pami_metadata_t)  );
  pami_algorithm_t * fast_alltoall_algs = (pami_algorithm_t *) safemalloc( num_alltoall_alg[1] * sizeof(pami_algorithm_t) );
  pami_metadata_t  * fast_alltoall_meta = (pami_metadata_t  *) safemalloc( num_alltoall_alg[1] * sizeof(pami_metadata_t)  );
  result = PAMI_Geometry_algorithms_query( world_geometry, alltoall_xfer,
                                           safe_alltoall_algs, safe_alltoall_meta, num_alltoall_alg[0],
                                           fast_alltoall_algs, fast_alltoall_meta, num_alltoall_alg[1] );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

  /* perform a gather */
  volatile int active = 0;

  int max = (argc>1 ? atoi(argv[1]) : 1000);
  max /= world_size;

  for ( int d = 1; d < max ; d*=2 )
    for ( size_t b = 0 ; b < num_alltoall_alg[0] ; b++ )
    {
        pami_xfer_t alltoall;

        alltoall.cb_done   = cb_done;
        alltoall.cookie    = (void*) &active;
        alltoall.algorithm = safe_alltoall_algs[b];

        double * sbuf = safemalloc(world_size*d*sizeof(double));
        double * rbuf = safemalloc(world_size*d*sizeof(double));
        for (int s=0; s<world_size; s++ )
          for (int k=0; k<d; k++)
            sbuf[s*d+k] = world_rank*d+k;
        for (int s=0; s<world_size; s++ )
          for (int k=0; k<d; k++)
            rbuf[s*d+k] = -1.0;

        alltoall.cmd.xfer_alltoall.sndbuf     = (void*)sbuf;
        alltoall.cmd.xfer_alltoall.stype      = PAMI_TYPE_DOUBLE;
        alltoall.cmd.xfer_alltoall.stypecount = d;
        alltoall.cmd.xfer_alltoall.rcvbuf     = (void*)rbuf;
        alltoall.cmd.xfer_alltoall.rtype      = PAMI_TYPE_DOUBLE;
        alltoall.cmd.xfer_alltoall.rtypecount = d;

        active = 1;
        double t0 = PAMI_Wtime(client);
        result = PAMI_Collective( contexts[0], &alltoall );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - alltoall");
        while (active)
          result = PAMI_Context_advance( contexts[0], 1 );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance - alltoall");
        double t1 = PAMI_Wtime(client);

        for (int s=0; s<world_size; s++ )
          for (int k=0; k<d; k++)
          {
            double correct = 1.0*s*d+1.0*k;
            if (rbuf[s*d+k]!=correct) 
              printf("%4d: rbuf[%d] = %lf \n", (int)world_rank, s*d+k, rbuf[s*d+k] );
          }

        free(sbuf);
        free(rbuf);

        if ( world_rank == 0 ) printf("safe alltoall algorithm %ld (%s) - %d doubles took %lf seconds (%lf MB/s) \n",
                                       b, safe_alltoall_meta[b].name, (int)world_size*d, t1-t0, 1e-6*world_size*d*sizeof(double)/(t1-t0) );
        fflush(stdout);
    }

  for ( int d = 1; d < 512 ; d++ ) /* allreduce and allgather barf >496 bytes */
    for ( size_t b = 0 ; b < num_alltoall_alg[1] ; b++ )
    {
        pami_xfer_t alltoall;

        alltoall.cb_done   = cb_done;
        alltoall.cookie    = (void*) &active;
        alltoall.algorithm = fast_alltoall_algs[b];

        double * sbuf = safemalloc(world_size*d*sizeof(double));
        double * rbuf = safemalloc(world_size*d*sizeof(double));
        for (int s=0; s<world_size; s++ )
          for (int k=0; k<d; k++)
            sbuf[s*d+k] = world_rank*d+k;
        for (int s=0; s<world_size; s++ )
          for (int k=0; k<d; k++)
            rbuf[s*d+k] = -1.0;

        alltoall.cmd.xfer_alltoall.sndbuf     = (void*)sbuf;
        alltoall.cmd.xfer_alltoall.stype      = PAMI_TYPE_DOUBLE;
        alltoall.cmd.xfer_alltoall.stypecount = d;
        alltoall.cmd.xfer_alltoall.rcvbuf     = (void*)rbuf;
        alltoall.cmd.xfer_alltoall.rtype      = PAMI_TYPE_DOUBLE;
        alltoall.cmd.xfer_alltoall.rtypecount = d;

        active = 1;
        double t0 = PAMI_Wtime(client);
        result = PAMI_Collective( contexts[0], &alltoall );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Collective - alltoall");
        while (active)
          result = PAMI_Context_advance( contexts[0], 1 );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance - alltoall");
        double t1 = PAMI_Wtime(client);

        for (int s=0; s<world_size; s++ )
          for (int k=0; k<d; k++)
          {
            double correct = 1.0*s*d+1.0*k;
            if (rbuf[s*d+k]!=correct) 
              printf("%4d: rbuf[%d] = %lf \n", (int)world_rank, s*d+k, rbuf[s*d+k] );
          }

        free(sbuf);
        free(rbuf);

        if ( world_rank == 0 ) printf("fast alltoall algorithm %ld (%s) - %d doubles took %lf seconds (%lf MB/s) \n",
                                       b, fast_alltoall_meta[b].name, (int)world_size*d, t1-t0, 1e-6*world_size*d*sizeof(double)/(t1-t0) );
        fflush(stdout);
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

  return 0;
}
