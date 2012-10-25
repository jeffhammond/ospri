#include "pamid.h"

int PAMID_Get_endtoend(size_t bytes, void * local, size_t target, void * remote)
{
	pami_result_t rc = PAMI_ERROR;

	pami_endpoint_t target_ep;
	result = PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client, (pami_task_t) target,
			PAMID_INTERNAL_STATE.context_roles.remote_get_context, &target_ep);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Endpoint_create");

	int active = 1;
	pami_get_simple_t parameters;
	parameters.rma.dest     = target_ep;
	//parameters.rma.hints    = ;
	parameters.rma.bytes    = bytes;
	parameters.rma.cookie   = &active;
	parameters.rma.done_fn  = cb_done;
	parameters.addr.local   = local;
	parameters.addr.remote  = remote;

	result = PAMI_Get(pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_blocking_context], &parameters);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Get");

	while (active)
	{
		result = PAMI_Context_trylock_advancev(pami_contexts, PAMID_INTERNAL_STATE.context_roles.local_blocking_context, 1000);
		PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
	}

	return PAMI_SUCCESS;
}

int PAMID_Get_nonblocking(size_t bytes, void * local, size_t target, void * remote, pamid_request_t * request)
{
	pami_result_t rc = PAMI_ERROR;

	pami_endpoint_t target_ep;
	result = PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client, (pami_task_t) target,
			PAMID_INTERNAL_STATE.context_roles.remote_get_context, &target_ep);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Endpoint_create");

	((*request).local)++;
	pami_get_simple_t parameters;
	parameters.rma.dest     = target_ep;
	//parameters.rma.hints    = ;
	parameters.rma.bytes    = bytes;
	parameters.rma.cookie   = request;
	parameters.rma.done_fn  = cb_local_done;
	parameters.addr.local   = local;
	parameters.addr.remote  = remote;

	result = PAMI_Get(pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_offload_context], &parameters);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Get");

	return PAMI_SUCCESS;
}
