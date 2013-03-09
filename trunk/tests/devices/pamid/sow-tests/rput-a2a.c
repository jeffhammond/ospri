#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>
#include <hwi/include/bqc/A2_inlines.h>

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

  world_size             = config[0].value.intval;
  world_rank             = config[1].value.intval;
  size_t num_contexts    = config[2].value.intval;
  size_t num_local_tasks = config[3].value.intval;
  TEST_ASSERT(num_contexts>1,"num_contexts>1");

  int ppn    = (int)num_local_tasks;
  int nnodes = world_size/ppn;
  int mycore = world_size%nnodes;
  int mynode = (world_rank-mycore)/ppn;

  if (world_rank==0)
  {
    printf("hello world from rank %ld of %ld, node %d of %d, core %d of %d \n", 
           world_rank, world_size, mynode, nnodes, mycore, ppn );
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

#ifdef PROGRESS_THREAD
  int status = pthread_create(&Progress_thread, NULL, &Progress_function, NULL);
  TEST_ASSERT(status==0, "pthread_create");
#endif

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

    pami_memregion_t shared_mr;
    result = PAMI_Memregion_create(contexts[1], rbuf, bytes, &bytes_out, &shared_mr);
    TEST_ASSERT(result == PAMI_SUCCESS && bytes==bytes_out,"PAMI_Memregion_create");
 
    pami_memregion_t local_mr;
    result = PAMI_Memregion_create(contexts[0], sbuf, bytes, &bytes_out, &local_mr);
    TEST_ASSERT(result == PAMI_SUCCESS && bytes==bytes_out,"PAMI_Memregion_create");
 
    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    pami_endpoint_t * target_eps = (pami_endpoint_t *) safemalloc( world_size * sizeof(pami_endpoint_t) );
    for (int target=0; target<world_size; target++)
    {
        result = PAMI_Endpoint_create(client, (pami_task_t) target, 1 /* async context*/, &(target_eps[target]) );
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Endpoint_create");
    }

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    pami_memregion_t * shmrs = (pami_memregion_t *) safemalloc( world_size * sizeof(pami_memregion_t) );

    result = allgather(world_geometry, contexts[0], sizeof(pami_memregion_t), &shared_mr, shmrs);
    TEST_ASSERT(result == PAMI_SUCCESS,"allgather");

    if (world_rank==0) 
    {
        printf("starting A2A \n");
        fflush(stdout);
    }

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

#ifdef SEPARATE_COMPLETION
    done_t active = { .local  = world_size, 
                      .remote = world_size  };
#else
    int active = world_size;
#endif

    uint64_t t0 = GetTimeBase();

    for (int count=0; count<world_size; count++)
    {
        int t = world_rank+count;
        int target = t%world_size;

        //printf("%ld: attempting Rput to %ld (bytes=%ld,loff=%ld, roff=%ld) \n", 
        //       (long)world_rank, (long)target, bytes, n*sizeof(double),
        //       target*n*sizeof(double), world_rank*n*sizeof(double));
        //printf("%ld: attempting Rput to %ld \n", (long)world_rank, (long)target),
        //fflush(stdout);


        pami_rput_simple_t parameters;
        parameters.rma.dest           = target_eps[target];
        //parameters.rma.hints          = ;
        parameters.rma.bytes          = n*sizeof(double);
        parameters.rma.cookie         = &active;
#ifdef SEPARATE_COMPLETION
        parameters.rma.done_fn        = cb_done_local;
        parameters.put.rdone_fn       = cb_done_remote;
#else
        parameters.rma.done_fn        = NULL;
        parameters.put.rdone_fn       = cb_done;
#endif
        parameters.rdma.local.mr      = &local_mr;
        parameters.rdma.local.offset  = target*n*sizeof(double);
        parameters.rdma.remote.mr     = &shmrs[target];
        parameters.rdma.remote.offset = world_rank*n*sizeof(double);

        result = PAMI_Rput(contexts[0], &parameters);
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Rput");
    }

#ifdef SEPARATE_COMPLETION
    while (active.local>0)
    {
      result = PAMI_Context_trylock_advancev(&(contexts[0]), 1, 1000);
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }
#endif

    uint64_t t1 = GetTimeBase();
    double  dt1 = (t1-t0)*tic;

#ifdef SEPARATE_COMPLETION
    while (active.remote>0)
#else
    while (active>0)
#endif
    {
      result = PAMI_Context_trylock_advancev(&(contexts[0]), 1, 1000);
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }

    uint64_t t2 = GetTimeBase();
    double  dt2 = (t2-t0)*tic;

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    double megabytes = 1.e-6*bytes;

    printf("%ld: PAMI_Rput A2A: %ld bytes per rank, local %lf seconds (%lf MB/s), remote %lf seconds (%lf MB/s) \n", 
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

    result = PAMI_Memregion_destroy(contexts[0], &shared_mr);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Memregion_destroy");
 
    result = PAMI_Memregion_destroy(contexts[0], &local_mr);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Memregion_destroy");
 
    free(target_eps);
    free(shmrs);
    free(rbuf);
    free(sbuf);
  }

  /************************************************************************/

#ifdef PROGRESS_THREAD
  void * rv;

  status = pthread_cancel(Progress_thread);
  TEST_ASSERT(status==0, "pthread_cancel");

  status = pthread_join(Progress_thread, &rv);
  TEST_ASSERT(status==0, "pthread_join");
#endif

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

