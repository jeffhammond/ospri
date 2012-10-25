#include "pamid.h"

int PAMID_Put_endtoend(size_t count, void * local, size_t target, void * remote)
{
	pami_result_t rc = PAMI_ERROR;

	pami_endpoint_t target_ep;
	result = PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client, (pami_task_t) target,
			PAMID_INTERNAL_STATE.context_roles.remote_put_context, &target_ep);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Endpoint_create");

	int active = 2;
	pami_put_simple_t parameters;
	parameters.rma.dest     = target_ep;
	//parameters.rma.hints    = ;
	parameters.rma.bytes    = bytes;
	parameters.rma.cookie   = &active;
	parameters.rma.done_fn  = cb_done;
	parameters.addr.local   = local;
	parameters.addr.remote  = shptrs[target];
	parameters.put.rdone_fn = cb_done;

	result = PAMI_Put(contexts[0], &parameters);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Put");

	while (active)
	{
		result = PAMI_Context_trylock_advancev(contexts, 1, 1000);
		PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev");
	}

	return PAMI_SUCCESS;
}
