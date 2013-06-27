#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>
#include <hwi/include/bqc/A2_inlines.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "safemalloc.h"
#include "preamble.h"
#include "coll.h"

const double tic = 1.0/(1.6e9);

int main(int argc, char* argv[])
{
  pami_result_t result = PAMI_ERROR;

  /* initialize the second client */
  char * clientname = "";
  pami_client_t client;
  result = PAMI_Client_create(clientname, &client, NULL, 0);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_create");

  /* query properties of the client */
  pami_configuration_t config[4];
  config[0].name = PAMI_CLIENT_NUM_TASKS;
  config[1].name = PAMI_CLIENT_TASK_ID;
  config[2].name = PAMI_CLIENT_NUM_CONTEXTS;
  config[3].name = PAMI_CLIENT_NUM_LOCAL_TASKS;
  result = PAMI_Client_query(client, config, 4);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  const size_t world_size      = config[0].value.intval;
  const size_t world_rank      = config[1].value.intval;
  const size_t num_contexts    = (config[2].value.intval > 32) ? 32 : config[2].value.intval; /* because I only need 16+16 contexts in c1 mode */
  const size_t num_local_tasks = config[3].value.intval;
  TEST_ASSERT(num_contexts>1,"num_contexts>1");

  const int ppn    = (int)num_local_tasks;
  const int nnodes = world_size/ppn;
  const int mycore = world_size%nnodes;
  const int mynode = (world_rank-mycore)/ppn;

  const int num_sync            = num_contexts/2;
  const int num_async           = num_contexts/2;
  const int async_context_begin = num_sync+1;
  const int async_context_end   = num_contexts;

  if (world_rank==0)
  {
    printf("hello world from rank %ld of %ld, node %d of %d, core %d of %d \n", 
           world_rank, world_size, mynode, nnodes, mycore, ppn );
    printf("num_contexts = %ld, async_context_begin = %d, async_context_end = %d \n",
            num_contexts, async_context_begin, async_context_end);
    fflush(stdout);
  }

  /* initialize the contexts */
  contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, NULL, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  /* setup the world geometry */
  pami_geometry_t world_geometry;
  result = PAMI_Geometry_world(client, &world_geometry );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Geometry_world");

  /************************************************************************/

  for (int n=1; n<=(256*1024); n*=2)
  {
    if (world_rank==0) 
    {
        printf("starting n = %d \n", n);
        fflush(stdout);
    }

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    double * sbuf = safemalloc(world_size*n*sizeof(double));
    double * rbuf = safemalloc(world_size*n*sizeof(double));

    for (int s=0; s<world_size; s++ )
      for (int k=0; k<n; k++)
        sbuf[s*n+k] = world_rank*n+k;

    for (int s=0; s<world_size; s++ )
      for (int k=0; k<n; k++)
        rbuf[s*n+k] = -1.0;

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    size_t bytes = world_size * n * sizeof(double), bytes_out;

    pami_memregion_t * local_mr  = safemalloc(num_sync * sizeof(pami_memregion_t) );
    pami_memregion_t * shared_mr = safemalloc(num_sync * sizeof(pami_memregion_t) );
    for (int i=0; i<num_sync; i++)
    {
        result = PAMI_Memregion_create(contexts[i], rbuf, bytes, &bytes_out, &(local_mr[i]));
        TEST_ASSERT(result == PAMI_SUCCESS && bytes==bytes_out,"PAMI_Memregion_create");

        result = PAMI_Memregion_create(contexts[async_context_begin+i], sbuf, bytes, &bytes_out, &(shared_mr[i]));
        TEST_ASSERT(result == PAMI_SUCCESS && bytes==bytes_out,"PAMI_Memregion_create");
    }
 
    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    pami_endpoint_t * target_eps = (pami_endpoint_t *) safemalloc( num_async * world_size * sizeof(pami_endpoint_t) );
    for (int target=0; target<world_size; target++)
        for (int i=0; i<num_async; i++)
        {
            result = PAMI_Endpoint_create(client, (pami_task_t) target, i, &(target_eps[target*num_async+i]) );
            TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Endpoint_create");
        }

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    pami_memregion_t * shmrs = (pami_memregion_t *) safemalloc( num_async * world_size * sizeof(pami_memregion_t) );

    result = allgather(world_geometry, contexts[0], num_async * sizeof(pami_memregion_t), shared_mr, shmrs);
    TEST_ASSERT(result == PAMI_SUCCESS,"allgather");

    /* check now that count will not iterate over an incomplete iteration space */
    int remote_targets_per_thread = world_size/num_sync;
    assert((world_size%num_sync)==0);

    if (world_rank==0) 
    {
        printf("starting A2A \n");
        fflush(stdout);
    }

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    int active = world_size;

    uint64_t t0 = GetTimeBase();

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    /* GCC prior to 4.7 will not permit const variables to be private i.e. firstprivate */
#ifdef _OPENMP
#pragma omp parallel default(shared) firstprivate(n, num_async, num_sync)
#endif
    {
#ifdef _OPENMP
        int tid = omp_get_thread_num();
#else
        int tid = 0;
#endif

        for (int count=0; count<remote_targets_per_thread; count++)
        {
          int target = remote_targets_per_thread*tid + count;
          target += world_rank;
          target  = target % world_size;
 
          //printf("%ld: attempting Rget to %ld \n", (long)world_rank, (long)target);
          //fflush(stdout);
          
          int local_context  = tid; /* each thread uses its own context so this is thread-safe */
          int remote_context = target % num_async;

          pami_rget_simple_t parameters;
          parameters.rma.dest           = target_eps[target*num_async+remote_context];
          //parameters.rma.hints          = ;
          parameters.rma.bytes          = n*sizeof(double);
          parameters.rma.cookie         = &active;
          parameters.rma.done_fn        = cb_done;
          parameters.rdma.local.mr      = &local_mr[local_context];
          parameters.rdma.local.offset  = target*n*sizeof(double);
          parameters.rdma.remote.mr     = &shmrs[target*num_async+remote_context];
          parameters.rdma.remote.offset = world_rank*n*sizeof(double);
 
          result = PAMI_Rget(contexts[local_context], &parameters);
          TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Rget");
        }
    }

    uint64_t t1 = GetTimeBase();
    double  dt1 = (t1-t0)*tic;

    while (active>0)
    {
      result = PAMI_Context_trylock_advancev(&(contexts[0]), num_sync+num_async, 1000);
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    uint64_t t2 = GetTimeBase();
    double  dt2 = (t2-t0)*tic;

    //result = barrier(world_geometry, contexts[0]);
    //TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    double megabytes = 1.e-6*bytes;

    printf("%ld: PAMI_Rget A2A: %ld bytes per rank, local %lf seconds (%lf MB/s), remote %lf seconds (%lf MB/s) \n", 
           (long)world_rank, n*sizeof(double), 
           dt1, megabytes/dt1,
           dt2, megabytes/dt2 );
    fflush(stdout);

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    for (int s=0; s<world_size; s++ )
      for (int k=0; k<n; k++)
      {
        if (rbuf[s*n+k]!=(1.0*s*n+1.0*k))
          printf("%4d: rbuf[%d] = %lf (%lf) \n", (int)world_rank, s*n+k, rbuf[s*n+k], (1.0*s*n+1.0*k) );
      }
    fflush(stdout);

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    for (int i=0; i<num_async; i++)
    {
        result = PAMI_Memregion_destroy(contexts[i], &(local_mr[i]) );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Memregion_destroy");

        result = PAMI_Memregion_destroy(contexts[async_context_begin+i], &(shared_mr[i]) );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Memregion_destroy");
    }
 
    free(shared_mr);
    free(local_mr);
    free(target_eps);
    free(shmrs);
    free(rbuf);
    free(sbuf);
  }

  /************************************************************************/

  result = barrier(world_geometry, contexts[0]);
  TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

  /* finalize the contexts */
  result = PAMI_Context_destroyv( contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_destroyv");

  free(contexts);

  /* finalize the client */
  result = PAMI_Client_destroy( &client );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_destroy");

  if (world_rank==0)
    printf("%ld: end of test \n", world_rank );
  fflush(stdout);

  return 0;
}

