#include "pamid.h"

int PAMID_Broadcast_setup(pami_geometry_t geometry, pamid_collective_state_t * broadcast)
{
	pami_result_t rc = PAMI_ERROR;

	broadcast->xfer = PAMI_XFER_BROADCAST;

	/* query the geometry */
	rc = PAMI_Geometry_algorithms_num(geometry, broadcast->xfer, broadcast->num_alg );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");

	broadcast->safe_algs = (pami_algorithm_t *) PAMIU_Malloc( broadcast->num_alg[0] * sizeof(pami_algorithm_t) );
	broadcast->fast_algs = (pami_algorithm_t *) PAMIU_Malloc( broadcast->num_alg[1] * sizeof(pami_algorithm_t) );
	broadcast->safe_meta = (pami_metadata_t  *) PAMIU_Malloc( broadcast->num_alg[0] * sizeof(pami_metadata_t)  );
	broadcast->fast_meta = (pami_metadata_t  *) PAMIU_Malloc( broadcast->num_alg[1] * sizeof(pami_metadata_t)  );
	rc = PAMI_Geometry_algorithms_query(geometry,
			broadcast->xfer,
			broadcast->safe_algs,
			broadcast->safe_meta,
			broadcast->num_alg[0],
			broadcast->fast_algs,
			broadcast->fast_meta,
			broadcast->num_alg[1]);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

	return PAMI_SUCCESS;
}

int PAMID_Broadcast_teardown(pamid_collective_state_t * broadcast)
{
	PAMIU_Free(broadcast->safe_algs);
	PAMIU_Free(broadcast->fast_algs);
	PAMIU_Free(broadcast->safe_meta);
	PAMIU_Free(broadcast->fast_meta);

	return PAMI_SUCCESS;
}

int PAMID_Broadcast_doit(pamid_collective_state_t * broadcast, int root, size_t num_bytes, void * buffer)
{
	pami_result_t rc = PAMI_ERROR;

	size_t broadcast_alg = 0; /* 0 is not necessarily the best one... */

	pami_xfer_t this;
	volatile int active = 0;

	this.cb_done   = cb_done;
	this.cookie    = (void*) &active;
	this.algorithm = broadcast->safe_algs[broadcast_alg]; /* safe algs should (must?) work */

	pami_endpoint_t root_ep;
	PAMI_Endpoint_create(PAMID_INTERNAL_STATE.pami_client,
			(pami_task_t)0,
			PAMID_INTERNAL_STATE.context_roles.local_blocking_context,
			&root_ep);

	this.cmd.xfer_broadcast.root      = root_ep;
	this.cmd.xfer_broadcast.buf       = buffer;
	this.cmd.xfer_broadcast.type      = PAMI_TYPE_BYTE;
	this.cmd.xfer_broadcast.typecount = num_bytes;

	/* perform a broadcast */
	active = 1;
	rc = PAMI_Collective(PAMID_INTERNAL_STATE.pami_contexts[0], &this );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Collective - broadcast");

	while (active)
		rc = PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[0]), 1, 1000 );

	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev - broadcast");

	return PAMI_SUCCESS;
}

int PAMID_Broadcast_world(int root, size_t num_bytes, void * buffer)
{
	return PAMID_Broadcast_doit(&(PAMID_INTERNAL_STATE.world_bcast), root, num_bytes, buffer);
}
