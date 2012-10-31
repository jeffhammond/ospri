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
  pami_result_t result = PAMI_ERROR;

  /* initialize the client */
  pami_client_t client;
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

  /* setup the world geometry */
  pami_geometry_t world_geometry;
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

  return 0;
}

