#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

const size_t poll_continuous = -1;

//  typedef void (*pami_dispatch_p2p_function) (pami_context_t    context,
//                                              void            * cookie,
//                                              const void      * header_addr,
//                                              size_t            header_size,
//                                              const void      * pipe_addr,
//                                              size_t            data_size,
//                                              pami_endpoint_t   origin,
//                                              pami_recv_t     * recv);

//void cb_recv_new(pami_context_t context,
//                 void * clientdata,
//                 void * header_addr,
//                 size_t header_size,
//                 void * pipe_addr,
//                 size_t pipe_size,
//                 pami_endpoint_t origin,
//                 pami_recv_t * recv)
//{
//
//    return;
//}

void dispatch_foo(pami_context_t    context,
                  void            * cookie,
                  const void      * hbase,
                  size_t            hlen,
                  const void      * dbase,
                  size_t            dlen,
                  pami_endpoint_t   origin,
                  pami_recv_t     * recv)
{
    volatile size_t * active = (volatile size_t *) cookie;
    int64_t * header_base    = (int64_t *) hbase;
    uint32_t  header_len     = (uint32_t)  hlen;
    int64_t * data_base      = (int64_t *) dbase;
    uint32_t  data_len       = (uint32_t)  dlen;

    printf("dispatch_foo from endpoint %ld, header_len = %ld, data_len = %ld",
           (long) origin, (long) header_len, (long) data_len);

    size_t i;
    for ( i=0 ; i<header_len ; i++ )
        printf("header_base[%ld] = %ld \n", i, header_base[i]);

    for ( i=0 ; i<data_len ; i++ )
        printf("data_base[%ld] = %ld \n", i, data_base[i]);

    (*active)--;

    fflush(stdout);

    return;
}

int main(int argc, char* argv[])
{
    pami_result_t        result        = PAMI_ERROR;

    /* initialize the client */
    char * clientname = "";
    pami_client_t client;
    result = PAMI_Client_create (clientname, &client, NULL, 0);
    assert(result == PAMI_SUCCESS);

    /* query properties of the client */
    pami_configuration_t config;
    int world_size, world_rank;

    config.name = PAMI_CLIENT_NUM_TASKS;
    result = PAMI_Client_query( client, &config,1);
    assert(result == PAMI_SUCCESS);
    world_size = config.value.intval;

    config.name = PAMI_CLIENT_TASK_ID;
    result = PAMI_Client_query( client, &config,1);
    assert(result == PAMI_SUCCESS);
    world_rank = config.value.intval;

    if (world_rank==0)
        printf("hello world from rank %d of %d \n", world_rank, world_size);
    fflush(stdout);

    int num_contexts = 1;
#ifdef MULTICONTEXT
    config.name = PAMI_CLIENT_NUM_CONTEXTS;
    result = PAMI_Client_query( client, &config, 1);
    assert(result == PAMI_SUCCESS);
    num_contexts = config.value.intval;
#endif

    /* initialize the contexts */
    pami_context_t * contexts = NULL;
    contexts = (pami_context_t *) malloc( num_contexts * sizeof(pami_context_t) );

    result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts);
    assert(result == PAMI_SUCCESS);

    if (world_rank==0)
        printf("%d contexts were created \n", num_contexts);
    fflush(stdout);

    /*************************************************************************
     * setup dispatch
     *************************************************************************/

    size_t dispatch_id = 1;
    pami_dispatch_callback_function dispatch_fn;
    dispatch_fn.p2p = dispatch_foo;
    int32_t active;
    pami_dispatch_hint_t dispatch_hint;
    memset(&dispatch_hint, 0x00, sizeof(pami_dispatch_hint_t));
//    dispatch_hint.remote_async_progress = PAMI_HINT_ENABLE;
//    isend_params.dispatch        = dispatch_hint;

    result = PAMI_Dispatch_set(contexts[0], dispatch_id, dispatch_fn, &active, dispatch_hint);
    assert(result == PAMI_SUCCESS);

    /*************************************************************************
     * setup endpoints (and timing array)
     *************************************************************************/

    uint32_t i;
    pami_endpoint_t * ep_list = malloc(world_size * sizeof(pami_endpoint_t));
    for ( i=0 ; i<world_size ; i++ )
    {
        result = PAMI_Endpoint_create(client, (pami_task_t)i, (size_t)0, &ep_list[i]);
        assert(result == PAMI_SUCCESS);
    }

    double * dt_list = malloc(world_size * sizeof(double));

    /*************************************************************************
     * invoke isend
     *************************************************************************/

    pami_send_immediate_t isend_params;

    uint32_t header_len = 4;
    int64_t header_base[header_len];
    for ( i=0 ; i<header_len ; i++ )
        header_base[i] = 0;

    uint32_t data_len = 1024;
    int64_t data_base[data_len];
    for ( i=0 ; i<data_len ; i++ )
        data_base[i] = i;

    uint32_t j;
    for ( j=0 ; j<data_len ; j*=2 )
    {
        isend_params.header.iov_len  = (size_t) header_len;
        isend_params.header.iov_base = (char *) header_base;

        isend_params.data.iov_len  = (size_t) data_len;
        isend_params.data.iov_base = (char *) data_base;

        for ( i=0 ; i<world_size ; i++ )
        {
            active = 1;

            isend_params.dest = ep_list[i];

            double t0 = PAMI_Wtime(client);

            result = PAMI_Send_immediate(contexts[0], &isend_params);
            assert(result == PAMI_SUCCESS);

            while (active)
            {
                result = PAMI_Context_advance(contexts[0], poll_continuous);
                assert(result == PAMI_SUCCESS);
            }

            double t1 = PAMI_Wtime(client);

            dt_list[i] = t1-t0;
        }

        for ( i=0 ; i<world_size ; i++ )
            printf("sent %d bytes to rank %d in %lf seconds = %lf MB/s",
                   (int) data_len, (int) i, dt_list[i], 1e-6 * data_len/dt_list[i]);
    }

    /*************************************************************************
     * cleanup endpoints (and timing array)
     *************************************************************************/

    free(ep_list);
    free(dt_list);

    /*************************************************************************/

    result = PAMI_Context_destroyv(contexts, num_contexts);
    assert(result == PAMI_SUCCESS);

    free(contexts);

    result = PAMI_Client_destroy(&client);
    assert(result == PAMI_SUCCESS);

    printf("end of test \n");
    fflush(stdout);

    return 0;
}

