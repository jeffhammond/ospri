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
    recv->type        = PAMI_TYPE_DOUBLE;
    recv->offset      = 0;
    recv->data_fn     = PAMI_DATA_SUM;
    recv->data_cookie = NULL;
  }

  return;
}

int init(void)
{
  pami_result_t result = PAMI_ERROR;

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
  TEST_ASSERT(num_contexts>1,"num_contexts>1");

  if (world_rank==0)
  {
    printf("hello world from rank %ld of %ld \n", world_rank, world_size );
    fflush(stdout);
  }

  /* initialize the contexts */
  pami_context_t * contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );

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

  /* register the dispatch function */
  pami_dispatch_callback_function dispatch_cb;
  size_t dispatch_id                 = 37;
  dispatch_cb.p2p                    = dispatch_recv_cb;
  pami_dispatch_hint_t dispatch_hint = {0};
  int dispatch_cookie                = 1000000+world_rank;
  //dispatch_hint.recv_immediate       = PAMI_HINT_DISABLE;
  result = PAMI_Dispatch_set(contexts[0], dispatch_id, dispatch_cb, &dispatch_cookie, dispatch_hint);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Dispatch_set");
  result = PAMI_Dispatch_set(contexts[1], dispatch_id, dispatch_cb, &dispatch_cookie, dispatch_hint);
  TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Dispatch_set");

  return 0;

int pami(void)
{
    int target = (world_rank>0 ? world_rank-1 : world_size-1);
    pami_endpoint_t target_ep;
    result = PAMI_Endpoint_create(client, (pami_task_t) target, 1, &target_ep);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Endpoint_create");

    result = barrier(world_geometry, contexts[0]);
    TEST_ASSERT(result == PAMI_SUCCESS,"barrier");

    int active = 2;
    pami_send_t parameters;
    parameters.send.header.iov_base = NULL;
    parameters.send.header.iov_len  = sizeof(void *);
    parameters.send.data.iov_base   = NULL;
    parameters.send.data.iov_len    = 0;
    parameters.send.dispatch        = dispatch_id;
    //parameters.send.hints           = ;
    parameters.send.dest            = target_ep;
    parameters.events.cookie        = &active;
    parameters.events.local_fn      = cb_done;
    parameters.events.remote_fn     = cb_done;

    result = PAMI_Send(contexts[0], &parameters);
    TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Send");

    while (active>1)
    {
      result = PAMI_Context_trylock_advancev(&(contexts[0]), 1, 1000);
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }

    while (active>0)
    {
      result = PAMI_Context_trylock_advancev(&(contexts[0]), 1, 1000);
      TEST_ASSERT(result == PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }

  }

  return 0;
}

