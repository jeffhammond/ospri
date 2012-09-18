#include "pamid.h"

pamid_global_state_t PAMID_INTERNAL_STATE = {0}; /* Dave or Jim knows why the "{0}" is used */

int PAMID_Initialize(void)
{
	pami_result_t rc = PAMI_ERROR;

	/* initialize the client */
	char * clientname = "";
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

	/* setup the world geometry */
	rc = PAMI_Geometry_world(PAMID_INTERNAL_STATE.pami_client, &(PAMID_INTERNAL_STATE.world_geometry) );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_world");

	rc = PAMID_Barrier_setup(PAMID_INTERNAL_STATE.world_geometry, &(PAMID_INTERNAL_STATE.world_barrier));
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMID_Barrier_setup");

	return PAMI_SUCCESS;
}

int PAMID_Finalize(void)
{
	pami_result_t rc = PAMI_ERROR;

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

