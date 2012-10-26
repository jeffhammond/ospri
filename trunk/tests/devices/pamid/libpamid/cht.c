#include "pamid.h"

pthread_t PAMID_Progress_thread;

void * PAMID_Progress_function(void * dummy)
{
	pami_result_t rc = PAMI_ERROR;

	printf("PAMID_Progress_function started");

	while (1)
	{
		/* advance all contexts except the local blocking one */
		for (int context = 1; context < PAMID_INTERNAL_STATE.num_contexts; context++)
		{
			rc = PAMI_Context_lock(PAMID_INTERNAL_STATE.pami_contexts[context]);
			PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_lock");

			rc = PAMI_Context_advance( PAMID_INTERNAL_STATE.pami_contexts[context], 1000 );
			PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_advance");

			rc = PAMI_Context_unlock(PAMID_INTERNAL_STATE.pami_contexts[context]);
			PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_unlock");
		}
	}
	return NULL;
}
