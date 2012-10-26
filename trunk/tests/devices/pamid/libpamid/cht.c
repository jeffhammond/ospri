#include "pamid.h"

pthread_t PAMID_Progress_thread;

void * PAMID_Progress_function(void * dummy)
{
	if (PAMID_INTERNAL_STATE.num_contexts>1)
		while (1)
		{
			/* advance all contexts except the local blocking one */
			rc = PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_offload_context]),
					PAMID_INTERNAL_STATE.num_contexts-1, 1000 );
			PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev - CHT");
		}
	else
		while (1)
		{
			/* only one context exists, we must advance it */
			rc = PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_blocking_context]),
					1, 1000 );
			PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev - CHT");
		}

	return NULL;
}
