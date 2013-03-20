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

static void dispatch_done_cb(pami_context_t context, void * cookie, pami_result_t result)
{
  return;
}

static void dispatch_recv_cb(pami_context_t context,
                             void * cookie,
                             const void * header_addr, size_t header_size,
                             const void * pipe_addr,
                             size_t data_size,
                             pami_endpoint_t origin,
                             pami_recv_t * recv)
{
  void ** h = (void **)header_addr;

  if (pipe_addr!=NULL)
  {
    memcpy(*h, pipe_addr, data_size);
  }
  else
  {
    recv->cookie      = 0;
    recv->local_fn    = NULL;
    recv->addr        = *h;
    recv->type        = PAMI_TYPE_BYTE;
    recv->offset      = 0;
    recv->data_fn     = PAMI_DATA_COPY;
    recv->data_cookie = NULL;
  }

  return;
}

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
  const size_t num_contexts    = config[2].value.intval>32 ? 32 : config[2].value.intval; /* because I only need 16+16 contexts in c1 mode */
  const size_t num_local_tasks = config[3].value.intval;
  TEST_ASSERT(num_contexts>1,"num_contexts>1");

  const int ppn    = (int)num_local_tasks;
  const int nnodes = world_size/ppn;
  const int mycore = world_size%nnodes;
  const int mynode = (world_rank-mycore)/ppn;

  const int num_sync            = num_contexts/2;
  const int num_async           = num_contexts/2;

  if (world_rank==0)
  {
    printf("hello world from rank %ld of %ld \n", world_rank, world_size );
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

  /* register the dispatch function */
  size_t dispatch_id = 37;

  pami_dispatch_callback_function dispatch_cb   = { .p2p = dispatch_recv_cb };
  pami_dispatch_hint_t            dispatch_hint = { .recv_immediate = PAMI_HINT_ENABLE };

  for (int i=0; i<num_contexts; i++)
  {
    int dispatch_cookie = world_rank*num_contexts+i;
    result = PAMI_Dispatch_set(contexts[i], 
                               dispatch_id, 
                               dispatch_cb, 
                               &dispatch_cookie, 
                               dispatch_hint);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Dispatch_set");
  }

  /************************************************************************/

  size_t n = 16*1024*1024;
  size_t bytes  = n * sizeof(int64_t);
  int64_t * shared = (int64_t *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    shared[i] = -1;

  int *  local  = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    local[i] = world_rank;

  int64_t ** shptrs = (int64_t **) safemalloc( world_size * sizeof(int64_t *) );

  result = allgather(world_geometry, contexts[0], sizeof(int64_t*), &shared, shptrs);
  TEST_ASSERT(result == PAMI_SUCCESS,"allgather");

  int target = (world_rank>0 ? world_rank-1 : world_size-1);
  pami_endpoint_t target_endpoints * = safemalloc(num_contexts * sizeof(pami_endpoint_t) );
  for (int i=0; i<num_contexts; i++)
  {
      result = PAMI_Endpoint_create(client, (pami_task_t) target, i, &(target_endpoints[i]));
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Endpoint_create");
  }

  result = barrier(world_geometry, contexts[0]);
  TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

  pami_send_immediate_t parameters = { .header.iov_base = &(shptrs[target]),
                                       .header.iov_len  = sizeof(int64_t *),
                                       .data.iov_base   = local,
                                       .data.iov_len    = bytes,
                                       .dispatch        = dispatch_id,
                                       .dest            = target_ep };

  uint64_t t0 = GetTimeBase();

#ifdef _OPENMP
#pragma omp parallel default(shared) firstprivate(n, num_async, num_sync)
#endif
  {
    int tid = omp_get_thread_num();

    #pragma omp parallel for
    for (int i=0; i<n; i++)
    {
        parameters.iov_base = &(shptrs[target])+i;
        result = PAMI_Send_immediate(contexts[tid], &parameters);
        TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Send");
    }
  }

  uint64_t t1 = GetTimeBase();
  uint64_t dt = t1-t0;

  printf("%ld: PAMI_Send_immediate of %ld bytes achieves %lf MB/s \n", (long)world_rank, bytes, 1.6e9*1e-6*(double)bytes/(double)dt );
  fflush(stdout);

  int errors = 0;
  
  target = (world_rank<(world_size-1) ? world_rank+1 : 0);
  for (int i=0; i<n; i++)
    if (shared[i] != target)
       errors++;

  if (errors>0)
    for (int i=0; i<n; i++)
      if (shared[i] != target)
        printf("%ld: shared[%d] = %d (%d) \n", (long)world_rank, i, shared[i], target);
  else
    printf("%ld: no errors :-) \n", (long)world_rank); 

  fflush(stdout);

  if (errors>0)
    exit(13);

  result = barrier(world_geometry, contexts[0]);
  TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

  free(shptrs);
  free(local);
  free(shared);

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

