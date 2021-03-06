#include "pamid.h"

int PAMID_Put_endtoend(size_t bytes, void * local, size_t target, void * remote)
{
    pami_result_t result = PAMI_ERROR;

    pami_endpoint_t target_ep;
    result = PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client, (pami_task_t) target,
            PAMID_INTERNAL_STATE.context_roles.remote_put_context, &target_ep);
    PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Endpoint_create");

    pamid_request_t request;
    request.local  = 1;
    request.remote = 1;
    pami_put_simple_t parameters;
    parameters.rma.dest     = target_ep;
    //parameters.rma.hints    = ;
    parameters.rma.bytes    = bytes;
    parameters.rma.cookie   = &request;
    parameters.rma.done_fn  = cb_local_done;
    parameters.addr.local   = local;
    parameters.addr.remote  = remote;
    parameters.put.rdone_fn = cb_remote_done;

    result = PAMI_Put(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_offload_context], &parameters);
    PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Put");

    while (request.local>0)
    {
        result = PAMI_Context_trylock_advancev(PAMID_INTERNAL_STATE.pami_contexts, PAMID_INTERNAL_STATE.context_roles.local_blocking_context, 1000);
        //PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }
    while (request.remote>0)
    {
        result = PAMI_Context_trylock_advancev(PAMID_INTERNAL_STATE.pami_contexts, PAMID_INTERNAL_STATE.context_roles.local_blocking_context, 1000);
        //PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }
    return PAMI_SUCCESS;
}

int PAMID_Put_halfway(size_t bytes, void * local, size_t target, void * remote, pamid_request_t * request)
{
    pami_result_t result = PAMI_ERROR;

    pami_endpoint_t target_ep;
    result = PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client, (pami_task_t) target,
            PAMID_INTERNAL_STATE.context_roles.remote_put_context, &target_ep);
    PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Endpoint_create");

    request->local  = 1;
    request->remote = 1;
    pami_put_simple_t parameters;
    parameters.rma.dest     = target_ep;
    //parameters.rma.hints    = ;
    parameters.rma.bytes    = bytes;
    parameters.rma.cookie   = request;
    parameters.rma.done_fn  = cb_local_done;
    parameters.addr.local   = local;
    parameters.addr.remote  = remote;
    parameters.put.rdone_fn = cb_remote_done;

    result = PAMI_Put(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_offload_context], &parameters);
    PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Put");

    while (((*request).local)>0)
    {
        result = PAMI_Context_trylock_advancev(PAMID_INTERNAL_STATE.pami_contexts, PAMID_INTERNAL_STATE.context_roles.local_blocking_context, 1000);
        //PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
    }

    return PAMI_SUCCESS;
}

int PAMID_Put_nonblocking(size_t bytes, void * local, size_t target, void * remote, pamid_request_t * request)
{
    pami_result_t result = PAMI_ERROR;

    pami_endpoint_t target_ep;
    result = PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client, (pami_task_t) target,
            PAMID_INTERNAL_STATE.context_roles.remote_put_context, &target_ep);
    PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Endpoint_create");

    request->local  = 1;
    request->remote = 1;
    pami_put_simple_t parameters;
    parameters.rma.dest     = target_ep;
    //parameters.rma.hints    = ;
    parameters.rma.bytes    = bytes;
    parameters.rma.cookie   = request;
    parameters.rma.done_fn  = cb_local_done;
    parameters.addr.local   = local;
    parameters.addr.remote  = remote;
    parameters.put.rdone_fn = cb_remote_done;

    result = PAMI_Put(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_offload_context], &parameters);
    PAMID_ASSERT(result==PAMI_SUCCESS,"PAMI_Put");

    return PAMI_SUCCESS;
}
