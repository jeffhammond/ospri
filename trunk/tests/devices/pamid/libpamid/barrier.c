#include "pamid.h"

int PAMID_Barrier_setup(pami_geometry_t geometry, pamid_collective_state_t * barrier)
{
	pami_result_t rc = PAMI_ERROR;

	barrier->xfer = PAMI_XFER_BARRIER;

	/* query the geometry */
	rc = PAMI_Geometry_algorithms_num( geometry, barrier->xfer, barrier->num_alg );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");

	barrier->safe_algs = (pami_algorithm_t *) PAMIU_Malloc( barrier->num_alg[0] * sizeof(pami_algorithm_t) );
	barrier->fast_algs = (pami_algorithm_t *) PAMIU_Malloc( barrier->num_alg[1] * sizeof(pami_algorithm_t) );
	barrier->safe_meta = (pami_metadata_t  *) PAMIU_Malloc( barrier->num_alg[0] * sizeof(pami_metadata_t)  );
	barrier->fast_meta = (pami_metadata_t  *) PAMIU_Malloc( barrier->num_alg[1] * sizeof(pami_metadata_t)  );
	rc = PAMI_Geometry_algorithms_query(geometry,
			barrier->xfer,
			barrier->safe_algs,
			barrier->safe_meta,
			barrier->num_alg[0],
			barrier->fast_algs,
			barrier->fast_meta,
			barrier->num_alg[1]);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

	return PAMI_SUCCESS;
}

int PAMID_Barrier_teardown(pamid_collective_state_t * barrier)
{
	PAMIU_Free(barrier->safe_algs);
	PAMIU_Free(barrier->fast_algs);
	PAMIU_Free(barrier->safe_meta);
	PAMIU_Free(barrier->fast_meta);

	return PAMI_SUCCESS;
}

int PAMID_Barrier_doit(pamid_collective_state_t * barrier)
{
	pami_result_t rc = PAMI_ERROR;

	size_t barrier_alg = 0; /* 0 is not necessarily the best one... */

	pami_xfer_t this;
	volatile int active = 0;

	this.cb_done   = cb_done;
	this.cookie    = (void*) &active;
	this.algorithm = barrier->safe_algs[barrier_alg]; /* safe algs should (must?) work */

	/* perform a barrier */
	active = 1;
	rc = PAMI_Collective( PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_blocking_context], &this );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Collective - barrier");

	while (active)
		rc = PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_blocking_context]),
				PAMID_INTERNAL_STATE.num_contexts, 1000 );

	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev - barrier");

	return PAMI_SUCCESS;
}

int PAMID_Barrier_world(void)
{
	return PAMID_Barrier_doit(&(PAMID_INTERNAL_STATE.world_barrier));
}
