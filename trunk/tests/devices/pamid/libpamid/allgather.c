#include "pamid.h"

int PAMID_Allgather_setup(pami_geometry_t geometry, pamid_collective_state_t * allgather)
{
	pami_result_t rc = PAMI_ERROR;

	allgather->xfer = PAMI_XFER_ALLGATHER;

	/* query the geometry */
	rc = PAMI_Geometry_algorithms_num(geometry, allgather->xfer, allgather->num_alg );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");

	allgather->safe_algs = (pami_algorithm_t *) PAMIU_Malloc( allgather->num_alg[0] * sizeof(pami_algorithm_t) );
	allgather->fast_algs = (pami_algorithm_t *) PAMIU_Malloc( allgather->num_alg[1] * sizeof(pami_algorithm_t) );
	allgather->safe_meta = (pami_metadata_t  *) PAMIU_Malloc( allgather->num_alg[0] * sizeof(pami_metadata_t)  );
	allgather->fast_meta = (pami_metadata_t  *) PAMIU_Malloc( allgather->num_alg[1] * sizeof(pami_metadata_t)  );
	rc = PAMI_Geometry_algorithms_query(geometry,
			allgather->xfer,
			allgather->safe_algs,
			allgather->safe_meta,
			allgather->num_alg[0],
			allgather->fast_algs,
			allgather->fast_meta,
			allgather->num_alg[1]);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

	return PAMI_SUCCESS;
}

int PAMID_Allgather_teardown(pamid_collective_state_t * allgather)
{
	PAMIU_Free(allgather->safe_algs);
	PAMIU_Free(allgather->fast_algs);
	PAMIU_Free(allgather->safe_meta);
	PAMIU_Free(allgather->fast_meta);

	return PAMI_SUCCESS;
}

int PAMID_Allgather_doit(pamid_collective_state_t * allgather,
                         size_t num_bytes, void * sbuf, void * rbuf)
{
	pami_result_t rc = PAMI_ERROR;

	size_t allgather_alg = 0; /* 0 is not necessarily the best one... */

	pami_xfer_t this;
	volatile int active = 1;

	this.cb_done   = cb_done;
	this.cookie    = (void*) &active;
	this.algorithm = allgather->safe_algs[allgather_alg]; /* safe algs should (must?) work */

    this.cmd.xfer_allgather.sndbuf     = (void*)sbuf;
    this.cmd.xfer_allgather.stype      = PAMI_TYPE_BYTE;
    this.cmd.xfer_allgather.stypecount = num_bytes;
    this.cmd.xfer_allgather.rcvbuf     = (void*)rbuf;
    this.cmd.xfer_allgather.rtype      = PAMI_TYPE_BYTE;
    this.cmd.xfer_allgather.rtypecount = num_bytes;

	/* perform a allgather */
	rc = PAMI_Collective(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_blocking_context], &this );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Collective");

	while (active)
		rc = PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[PAMID_INTERNAL_STATE.context_roles.local_blocking_context]), 1, 1000 );

	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev");

	return PAMI_SUCCESS;
}

int PAMID_Allgather_world(size_t num_bytes, void * sbuf, void * rbuf)
{
	return PAMID_Allgather_doit(&(PAMID_INTERNAL_STATE.world_allgather), num_bytes, sbuf, rbuf);
}
