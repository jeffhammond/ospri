#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>
#include <pami.h>
//#include <hwi/include/bqc/A2_inlines.h>

void * safemalloc(size_t n) 
{
    void * ptr = malloc( n );
    if ( ptr == NULL )
    {
        fprintf( stderr , "%ld bytes could not be allocated \n" , (long)n );
        exit(n);
    }
    return ptr;
}

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

  size_t bytes = 1000 * sizeof(int);
  int *  shared = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    shared[i] = -1;

  int *  local  = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    local[i] = world_rank;

  status = MPI_Barrier(MPI_COMM_WORLD);
  TEST_ASSERT(result == MPI_SUCCESS,"MPI_Barrier");

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

