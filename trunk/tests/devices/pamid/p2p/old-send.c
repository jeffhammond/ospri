#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

const size_t poll_continuous = -1;

int world_size, world_rank;

void cb_done(pami_context_t context, void * cookie, pami_result_t result)
{
    printf("%d: cb_done \n", world_rank);
    volatile int32_t * active = (volatile int32_t *) cookie;
    --(active);
    return;
}

void dispatch_cb(pami_context_t context,
                 void * cookie,
                 const void * header_addr,
                 size_t       header_size,
                 const void * pipe_addr,
                 size_t       pipe_size,
                 pami_endpoint_t origin,
                 pami_recv_t * recv)
{
    printf("%d: dispatch_cb from origin %ld \n", world_rank, (long) origin);
    volatile int64_t * active = (volatile int64_t *) cookie;
    --(*active);
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
    pami_dispatch_hint_t dispatch_hint = (pami_dispatch_hint_t) {0};
    pami_dispatch_callback_function dispatch_fn;
    dispatch_fn.p2p = dispatch_cb;
    int64_t dispatch_active;

    //    memset(&dispatch_hint, 0x00, sizeof(pami_dispatch_hint_t));
    //    dispatch_hint.remote_async_progress = PAMI_HINT_ENABLE;
    //    isend_params.dispatch        = dispatch_hint;

    dispatch_active = 1;
    result = PAMI_Dispatch_set(contexts[0], dispatch_id, dispatch_fn, &dispatch_active, dispatch_hint);
    assert(result == PAMI_SUCCESS);

    /*************************************************************************
     * invoke send
     *************************************************************************/

    if (world_rank==0)
    {
        double * dt_list = malloc(world_size * sizeof(double));

        uint32_t header_len = 4;
        int64_t header_base[header_len];
        for ( uint32_t i=0 ; i<header_len ; i++ )
            header_base[i] = 0;

        uint32_t data_len = 1024;
        int64_t data_base[data_len];
        for ( uint32_t i=0 ; i<data_len ; i++ )
            data_base[i] = i;

        for ( uint32_t j=1 ; j<data_len ; j*=2 )
        {
            int target;
            for ( target=1 ; target<world_size ; target++ )
            {
                int32_t active;
                active = 1;

                pami_send_t parameters;
                parameters.send.dispatch        = dispatch_id;
                parameters.send.header.iov_base = header_base;
                parameters.send.header.iov_len  = header_len;
                parameters.send.data.iov_base   = data_base;
                parameters.send.data.iov_len    = data_len;
                parameters.events.cookie        = (void *) &active;
                parameters.events.local_fn      = cb_done;
                parameters.events.remote_fn     = NULL;
                result = PAMI_Endpoint_create(client, (pami_task_t) target, (size_t) 0, &parameters.send.dest);
                assert(result == PAMI_SUCCESS);
                memset(&parameters.send.hints, 0, sizeof(parameters.send.hints));

                double t0 = PAMI_Wtime(client);
                {
                    result = PAMI_Send( contexts[0], &parameters);
                    assert(result == PAMI_SUCCESS);
                    while (active)
                    {
                        result = PAMI_Context_advance( contexts[0], poll_continuous);
                        assert(result == PAMI_SUCCESS);
                    }
                }
                double t1 = PAMI_Wtime(client);
                dt_list[target] = t1-t0;
            }

            for ( target=0 ; target<world_size ; target++ )
                printf("sent %d bytes to rank %d in %lf seconds = %lf MB/s",
                       (int) data_len, (int) target, dt_list[target], 1e-6 * data_len/dt_list[target]);
        }

        free(dt_list);
    }
    else
    {
        while (1)
        {
            result = PAMI_Context_advance( contexts[0], poll_continuous);
            assert(result == PAMI_SUCCESS);
        }
    }

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

