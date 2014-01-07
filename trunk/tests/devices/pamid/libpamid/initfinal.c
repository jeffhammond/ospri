#include "pamid.h"

pamid_global_state_t PAMID_INTERNAL_STATE = {0}; /* Dave or Jim knows why the "{0}" is used */

int PAMID_Initialize(void)
{
    pami_result_t rc = PAMI_ERROR;

    /* initialize the client */
    char * clientname = "PAMID";
    rc = PAMI_Client_create(clientname, &(PAMID_INTERNAL_STATE.pami_client), NULL, 0);
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Client_create");

    /* query properties of the client */
    pami_configuration_t config[3];
    config[0].name = PAMI_CLIENT_TASK_ID;
    config[1].name = PAMI_CLIENT_NUM_TASKS;
    config[2].name = PAMI_CLIENT_NUM_CONTEXTS;
    rc = PAMI_Client_query(PAMID_INTERNAL_STATE.pami_client, config, 3);
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Client_query");
    PAMID_INTERNAL_STATE.world_rank   = config[0].value.intval;
    PAMID_INTERNAL_STATE.world_size   = config[1].value.intval;
    PAMID_INTERNAL_STATE.num_contexts = config[2].value.intval;

    /* initialize the contexts */
    PAMID_INTERNAL_STATE.pami_contexts = (pami_context_t *) PAMIU_Malloc( PAMID_INTERNAL_STATE.num_contexts * sizeof(pami_context_t) );
    rc = PAMI_Context_createv(PAMID_INTERNAL_STATE.pami_client, config, 0, PAMID_INTERNAL_STATE.pami_contexts, PAMID_INTERNAL_STATE.num_contexts );
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_createv");

    if (PAMID_INTERNAL_STATE.num_contexts<2)
        if (PAMID_INTERNAL_STATE.world_rank==0)
        {
            printf("PAMID_Initialize: you need at least 2 contexts for this to work well (you have %ld) \n",
                    (long)PAMID_INTERNAL_STATE.num_contexts);
            fflush(stdout);
            sleep(1);
        }

    /* the sync contexts should come before the async ones */
    /* this is the synchronous context */
    PAMID_INTERNAL_STATE.context_roles.local_blocking_context = 0;
    /* these are all asynchronous contexts */
    PAMID_INTERNAL_STATE.context_roles.local_offload_context  = 1;
    PAMID_INTERNAL_STATE.context_roles.remote_put_context     = (PAMID_INTERNAL_STATE.num_contexts > 2 ? 2 : PAMID_INTERNAL_STATE.context_roles.local_offload_context);
    PAMID_INTERNAL_STATE.context_roles.remote_get_context     = (PAMID_INTERNAL_STATE.num_contexts > 3 ? 3 : PAMID_INTERNAL_STATE.context_roles.remote_put_context);
    PAMID_INTERNAL_STATE.context_roles.remote_acc_context     = (PAMID_INTERNAL_STATE.num_contexts > 4 ? 4 : PAMID_INTERNAL_STATE.context_roles.remote_get_context);
    PAMID_INTERNAL_STATE.context_roles.remote_rmw_context     = (PAMID_INTERNAL_STATE.num_contexts > 5 ? 5 : PAMID_INTERNAL_STATE.context_roles.remote_acc_context);

    /* TODO: if we want put, acc and/or rmw ordered w.r.t. each other, we need to set remote_put_context=remote_send_context */

    /* setup the world geometry */
    rc = PAMI_Geometry_world(PAMID_INTERNAL_STATE.pami_client, &(PAMID_INTERNAL_STATE.world_geometry) );
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_world");

    /* setup the world barrier */
    rc = PAMID_Barrier_setup(PAMID_INTERNAL_STATE.world_geometry, &(PAMID_INTERNAL_STATE.world_barrier));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_setup");

    /* setup the world broadcast */
    rc = PAMID_Broadcast_setup(PAMID_INTERNAL_STATE.world_geometry, &(PAMID_INTERNAL_STATE.world_bcast));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Broadcast_setup");

    /* setup the world allgather */
    rc = PAMID_Allgather_setup(PAMID_INTERNAL_STATE.world_geometry, &(PAMID_INTERNAL_STATE.world_allgather));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Allgather_setup");

    /* setup the world allreduce */
    rc = PAMID_Allreduce_setup(PAMID_INTERNAL_STATE.world_geometry, &(PAMID_INTERNAL_STATE.world_allreduce));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Allreduce_setup");

    rc = PAMID_Barrier_doit(&(PAMID_INTERNAL_STATE.world_barrier));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_doit A");

    if (PAMID_INTERNAL_STATE.num_contexts>1)
    {
        int status = pthread_create(&PAMID_Progress_thread, NULL, &PAMID_Progress_function, NULL);
        PAMID_ASSERT(status==0, "pthread_create");

        rc = PAMID_Barrier_doit(&(PAMID_INTERNAL_STATE.world_barrier));
        PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_doit B");
    }

    return PAMI_SUCCESS;
}

int PAMID_Finalize(void)
{
    pami_result_t rc = PAMI_ERROR;

    rc = PAMID_Barrier_doit(&(PAMID_INTERNAL_STATE.world_barrier));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_doit C");

    if (PAMID_INTERNAL_STATE.num_contexts>1)
    {
        int status = pthread_cancel(PAMID_Progress_thread);
        PAMID_ASSERT(status==0, "pthread_cancel");

        rc = PAMID_Barrier_doit(&(PAMID_INTERNAL_STATE.world_barrier));
        PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_doit D");
    }

    /* teardown the world allreduce */
    rc = PAMID_Allreduce_teardown(&(PAMID_INTERNAL_STATE.world_allreduce));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Allreduce_teardown");

    /* teardown the world allgather */
    rc = PAMID_Allgather_teardown(&(PAMID_INTERNAL_STATE.world_allgather));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Allgather_teardown");

    /* teardown the world broadcast */
    rc = PAMID_Broadcast_teardown(&(PAMID_INTERNAL_STATE.world_bcast));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Broadcast_teardown");

    /* teardown the world barrier */
    rc = PAMID_Barrier_teardown(&(PAMID_INTERNAL_STATE.world_barrier));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_teardown");

    /* finalize the contexts */
    rc = PAMI_Context_destroyv( PAMID_INTERNAL_STATE.pami_contexts, PAMID_INTERNAL_STATE.num_contexts );
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_destroyv");

    PAMIU_Free(PAMID_INTERNAL_STATE.pami_contexts);

    /* finalize the client */
    rc = PAMI_Client_destroy(&(PAMID_INTERNAL_STATE.pami_client));
    PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Client_destroy");

    return PAMI_SUCCESS;
}

size_t PAMID_World_rank(void)
{
    return PAMID_INTERNAL_STATE.world_rank;
}

size_t PAMID_World_size(void)
{
    return PAMID_INTERNAL_STATE.world_size;
}
