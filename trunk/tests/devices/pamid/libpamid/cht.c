#include "pamid.h"

int pamid_progress_active = 1;
pthread_t PAMID_Progress_thread;

void * PAMID_Progress_function(void * dummy)
{
    fprintf(stderr,"CHT: PAMID_Progress_function started \n");

    int c0 = PAMID_INTERNAL_STATE.context_roles.local_offload_context; 
    int c1 = PAMID_INTERNAL_STATE.context_roles.remote_rmw_context; 
        
    fprintf(stderr,"CHT: advancing contexts %d through %d \n", c0, c1);

    while (pamid_progress_active)
    {
#if 0
        /* advance all contexts except the local blocking one */
        for (int context  = PAMID_INTERNAL_STATE.context_roles.local_offload_context; 
                 context <= PAMID_INTERNAL_STATE.context_roles.remote_rmw_context; 
                 context++)
        {
            fprintf(stderr,"CHT: attempting to lock context %d \n", context);
            PAMI_Context_lock(PAMID_INTERNAL_STATE.pami_contexts[context]);
            PAMI_Context_advance( PAMID_INTERNAL_STATE.pami_contexts[context], 1000 );
            PAMI_Context_unlock(PAMID_INTERNAL_STATE.pami_contexts[context]);
            usleep(1);
        }
#endif
        PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[c0]), c1-c0 , 10 );
    }
    return NULL;
}

int PAMID_Progress_poke(void)
{
    int c0 = PAMID_INTERNAL_STATE.context_roles.local_offload_context; 
    int c1 = PAMID_INTERNAL_STATE.context_roles.remote_rmw_context; 

    PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[c0]), c1-c0 , 10 );

    return PAMI_SUCCESS;
}
