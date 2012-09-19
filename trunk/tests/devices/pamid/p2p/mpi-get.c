#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>
#include <pami.h>
#include <hwi/include/bqc/A2_inlines.h>

#include "safemalloc.h"

//#define SLEEP sleep
#define SLEEP usleep

#define PRINT_SUCCESS 1

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

int main(int argc, char* argv[])
{
  int status = MPI_SUCCESS; 
  pami_result_t result = PAMI_ERROR;

  int provided = MPI_THREAD_SINGLE;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided<MPI_THREAD_MULTIPLE) 
    exit(provided);

  /* initialize the second client */
  char * clientname = "";
  pami_client_t client;
  result = PAMI_Client_create(clientname, &client, NULL, 0);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_create");

  /* query properties of the client */
  pami_configuration_t config[3];
  size_t num_contexts;

  config[0].name = PAMI_CLIENT_NUM_TASKS;
  config[1].name = PAMI_CLIENT_TASK_ID;
  config[2].name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query(client, config, 3);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_query");
  world_size   = config[0].value.intval;
  world_rank   = config[1].value.intval;
  num_contexts = config[2].value.intval;

  if (world_rank==0)
    printf("hello world from rank %ld of %ld \n", world_rank, world_size );
  fflush(stdout);
  SLEEP(1);

  /* initialize the contexts */
  pami_context_t * contexts;
  contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, NULL, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  /************************************************************************/

  int n = (argc>1 ? atoi(argv[1]) : 1000);

  size_t bytes = n * sizeof(int);
  int *  shared = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    shared[i] = world_rank;

  int *  local  = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    local[i] = -1;

  status = MPI_Barrier(MPI_COMM_WORLD);
  TEST_ASSERT(result == MPI_SUCCESS,"MPI_Barrier");

  int ** shptrs = (int **) safemalloc( world_size * sizeof(int *) );

  status = MPI_Allgather(&shared, sizeof(int *), MPI_BYTE, 
                         shptrs,  sizeof(int *), MPI_BYTE, MPI_COMM_WORLD);
  TEST_ASSERT(result == MPI_SUCCESS,"MPI_Allgather");

  int target = (world_rank>0 ? world_rank-1 : world_size-1);
  pami_endpoint_t target_ep;
  result = PAMI_Endpoint_create(client, (pami_task_t) target, 0, &target_ep);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Endpoint_create");

  int active = 1;
  pami_get_simple_t parameters;
  parameters.rma.dest    = target_ep;
  parameters.rma.bytes   = bytes;
  parameters.rma.cookie  = &active;
  parameters.rma.done_fn = cb_done;
  parameters.addr.local  = local;
  parameters.addr.remote = shptrs[target];

  uint64_t t0 = GetTimeBase();

  result = PAMI_Get(contexts[0], &parameters);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Get");

  while (active)
  {
    //result = PAMI_Context_advance( contexts[0], 100);
    //TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");
    result = PAMI_Context_trylock_advancev(contexts, 1, 1000);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
  }

  uint64_t t1 = GetTimeBase();
  uint64_t dt = t1-t0;
  printf("%ld: PAMI_Get of %d bytes achieves %lf MB/s \n", (long)world_rank, n, 1.6e9*1e-6*(double)bytes/(double)dt );
  fflush(stdout);
  SLEEP(1);

  int errors = 0;
  
  for (int i=0; i<n; i++)
    if (local[i] != target)
       errors++;

  if (errors>0)
    for (int i=0; i<n; i++)
      printf("%ld: local[%d] = %d (%d) \n", (long)world_rank, i, local[i], target);
  else
    printf("%ld: no errors :-) \n", (long)world_rank); 

  fflush(stdout);
  SLEEP(1);
  MPI_Barrier(MPI_COMM_WORLD);
  free(shptrs);
  free(local);
  free(shared);

  /************************************************************************/

  /* finalize the contexts */
  result = PAMI_Context_destroyv( contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_destroyv");

  free(contexts);

  /* finalize the client */
  result = PAMI_Client_destroy( &client );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Client_destroy");

  status = MPI_Barrier(MPI_COMM_WORLD);
  TEST_ASSERT(result == MPI_SUCCESS,"MPI_Barrier");

  MPI_Finalize();

  if (world_rank==0)
    printf("%ld: end of test \n", world_rank );
  fflush(stdout);
  SLEEP(1);

  return 0;
}
