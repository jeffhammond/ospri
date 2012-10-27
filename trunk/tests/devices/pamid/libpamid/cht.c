#include "pamid.h"

pthread_t PAMID_Progress_thread;

void * PAMID_Progress_function(void * dummy)
{
	pami_result_t rc = PAMI_ERROR;

	//fprintf(stderr,"CHT: PAMID_Progress_function started \n");

	while (1)
	{
		/* advance all contexts except the local blocking one */
		//for (int context = 1; context < PAMID_INTERNAL_STATE.num_contexts; context++)
		{
			int context = 1;

			//fprintf(stderr,"CHT: attempting to lock context %d \n", context);

			rc = PAMI_Context_lock(PAMID_INTERNAL_STATE.pami_contexts[context]);
			//PAMID_ASSERT(rc==PAMI_SUCCESS,"CHT: PAMI_Context_lock");
			if (rc!=PAMI_SUCCESS) abort();

			rc = PAMI_Context_advance( PAMID_INTERNAL_STATE.pami_contexts[context], 1000 );
			//PAMID_ASSERT(rc==PAMI_SUCCESS,"CHT: PAMI_Context_advance");
			if (rc!=PAMI_SUCCESS) abort();

			rc = PAMI_Context_unlock(PAMID_INTERNAL_STATE.pami_contexts[context]);
			//PAMID_ASSERT(rc==PAMI_SUCCESS,"CHT: PAMI_Context_unlock");
			if (rc!=PAMI_SUCCESS) abort();

			usleep(1);
		}
	}
	return NULL;
}
