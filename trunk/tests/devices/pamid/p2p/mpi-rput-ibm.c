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
#include "preamble.h"
#include "coll.h"

int main(int argc, char* argv[])
{
  int status = MPI_SUCCESS; 
  pami_result_t result = PAMI_ERROR;

  int provided = MPI_THREAD_SINGLE;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  /* IBM: --ranks-per-node 64 fails to init threads but this  */
  /* IBM: testcase doesn't really care so don't exit */
  TEST_ASSERT((provided>=MPI_THREAD_MULTIPLE),"MPI_Init_thread"); 

  /* initialize the second client */
  char * clientname = "test"; /* IBM: PE PAMI requires a client name */
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
    printf("hello world from rank %ld of %ld, number of contexts %zu \n", world_rank, world_size, num_contexts );/*IBM: debug num_contexts */
  fflush(stdout);

  /* initialize the contexts */
  pami_context_t * contexts;
  contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );

  result = PAMI_Context_createv( client, NULL, 0, contexts, num_contexts );
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_createv");

  /************************************************************************/
  /* IBM: Updating the test with the assumption that we will Rput the     */
  /* IBM: local byte array to our neighbor's shared byte array.           */

  int n = (argc>1 ? atoi(argv[1]) : 1000);

  size_t bytes = n * sizeof(int), bytes_out;/* IBM: debug - scale up testing */
  int *  shared = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    shared[i] = -1;   /*IBM: initialize with -1, replaced with neighbor's rank */

  pami_memregion_t shared_mr;
  result = PAMI_Memregion_create(contexts[0], shared, bytes, &bytes_out, &shared_mr);
  TEST_ASSERT(result == PAMI_SUCCESS && bytes==bytes_out,"PAMI_Memregion_create");

  int *  local  = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    local[i] = world_rank;   /*IBM: initialize with our rank */

  pami_memregion_t local_mr;
  result = PAMI_Memregion_create(contexts[0], local, bytes, &bytes_out, &local_mr); /* IBM: local */
  TEST_ASSERT(result == PAMI_SUCCESS && bytes==bytes_out,"PAMI_Memregion_create");

  status = MPI_Barrier(MPI_COMM_WORLD);
  TEST_ASSERT(result == MPI_SUCCESS,"MPI_Barrier");

  pami_memregion_t * shmrs = (pami_memregion_t *) safemalloc( world_size * sizeof(pami_memregion_t) );

  status = MPI_Allgather(&shared_mr, sizeof(pami_memregion_t), MPI_BYTE, 
                         shmrs,      sizeof(pami_memregion_t), MPI_BYTE, MPI_COMM_WORLD);
  TEST_ASSERT(result == MPI_SUCCESS,"MPI_Allgather");

  int target = (world_rank>0 ? world_rank-1 : world_size-1);
  pami_endpoint_t target_ep;
  result = PAMI_Endpoint_create(client, (pami_task_t) target, 0, &target_ep);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Endpoint_create");

  int active = 2;
  pami_rput_simple_t parameters;
  parameters.rma.dest           = target_ep;
  parameters.rma.bytes          = bytes;
  parameters.rma.cookie         = &active;
  parameters.rma.done_fn        = cb_done;
  parameters.rdma.local.mr      = &local_mr;
  parameters.rdma.local.offset  = 0;
  parameters.rdma.remote.mr     = &shmrs[target]; /*IBM: target's mem region */
  parameters.rdma.remote.offset = 0;
  parameters.put.rdone_fn       = cb_done;
  result = PAMI_Rput(contexts[0], &parameters);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Rput");

  while (active)
  {
    //result = PAMI_Context_advance( contexts[0], 100);
    //TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_advance");
    result = PAMI_Context_trylock_advancev(contexts, 1, 1000);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
  }

  /* IBM: I'm done with Rput but my world_rank + 1 neighbor might not be so need to advance */
  /* IBM: Could do a barrier or send/recv a completion message instead ....*/

  active = 10; /* IBM: Arbitrary - advance some more - 10*10000 good enough?  */
  while (--active)                                                         /* IBM*/
  {                                                                        /* IBM*/
    result = PAMI_Context_trylock_advancev(contexts, 1, 10000);            /* IBM*/
  /*TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");*/ /* IBM*/
  }                                                                        /* IBM*/

  int errors = 0;
  
  target = (world_rank<(world_size-1) ? world_rank+1 : 0);
  for (int i=0; i<n; i++)
    if ((shared[i] != target) ||
        (local[i] != world_rank)) /*IBM: also verify didn't change local */
       errors++;

  if (errors>0)
  {
      printf("%ld: %d errors :-( \n", (long)world_rank,  errors);  /*IBM: grep "errors" in scaled up output */
      for (int i=0; i<n; i++)
        printf("%ld: local[%d] = %d , shared[%d] = %d (%d) \n", (long)world_rank, i, local[i], i, shared[i], target); /*IBM: print both arrays */
  }
  else
    printf("%ld: no errors :-) \n", (long)world_rank); 


  MPI_Barrier(MPI_COMM_WORLD);

  result = PAMI_Memregion_destroy(contexts[0], &shared_mr);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Memregion_destroy");

  result = PAMI_Memregion_destroy(contexts[0], &local_mr);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Memregion_destroy");

  free(shmrs);
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

  return 0;
}

