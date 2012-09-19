#include "pamid.h"

int PAMID_Allreduce_setup(pami_geometry_t geometry, pamid_collective_state_t * allreduce)
{
	pami_result_t rc = PAMI_ERROR;

	allreduce->xfer = PAMI_XFER_ALLREDUCE;

	/* query the geometry */
	rc = PAMI_Geometry_algorithms_num(geometry, allreduce->xfer, allreduce->num_alg );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");

	allreduce->safe_algs = (pami_algorithm_t *) PAMIU_Malloc( allreduce->num_alg[0] * sizeof(pami_algorithm_t) );
	allreduce->fast_algs = (pami_algorithm_t *) PAMIU_Malloc( allreduce->num_alg[1] * sizeof(pami_algorithm_t) );
	allreduce->safe_meta = (pami_metadata_t  *) PAMIU_Malloc( allreduce->num_alg[0] * sizeof(pami_metadata_t)  );
	allreduce->fast_meta = (pami_metadata_t  *) PAMIU_Malloc( allreduce->num_alg[1] * sizeof(pami_metadata_t)  );
	rc = PAMI_Geometry_algorithms_query(geometry,
			allreduce->xfer,
			allreduce->safe_algs,
			allreduce->safe_meta,
			allreduce->num_alg[0],
			allreduce->fast_algs,
			allreduce->fast_meta,
			allreduce->num_alg[1]);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

	return PAMI_SUCCESS;
}

int PAMID_Allreduce_teardown(pamid_collective_state_t * allreduce)
{
	PAMIU_Free(allreduce->safe_algs);
	PAMIU_Free(allreduce->fast_algs);
	PAMIU_Free(allreduce->safe_meta);
	PAMIU_Free(allreduce->fast_meta);

	return PAMI_SUCCESS;
}

int PAMID_Allreduce_doit(pamid_collective_state_t * allreduce,
                         size_t count, void * sbuf, void * rbuf, 
                         pami_type_t type, pami_data_function op)
{
	pami_result_t rc = PAMI_ERROR;

	size_t allreduce_alg = 0; /* 0 is not necessarily the best one... */

	pami_xfer_t this;
	volatile int active = 0;

	this.cb_done   = cb_done;
	this.cookie    = (void*) &active;
	this.algorithm = allreduce->safe_algs[allreduce_alg]; /* safe algs should (must?) work */

    this.cmd.xfer_allreduce.op         = op;
    this.cmd.xfer_allreduce.sndbuf     = (void*)sbuf;
    this.cmd.xfer_allreduce.stype      = type;
    this.cmd.xfer_allreduce.stypecount = count;
    this.cmd.xfer_allreduce.rcvbuf     = (void*)rbuf;
    this.cmd.xfer_allreduce.rtype      = type;
    this.cmd.xfer_allreduce.rtypecount = count;

	/* perform a allreduce */
	active = 1;
	rc = PAMI_Collective(PAMID_INTERNAL_STATE.pami_contexts[0], &this );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Collective - allreduce");

	while (active)
		rc = PAMI_Context_trylock_advancev( &(PAMID_INTERNAL_STATE.pami_contexts[0]), 1, 1000 );

	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev - allreduce");

	return PAMI_SUCCESS;
}

int PAMID_Allreduce_world(size_t count, void * sbuf, void * rbuf, pami_type_t type, pami_data_function op)
{
	return PAMID_Allreduce_doit(&(PAMID_INTERNAL_STATE.world_allreduce), count, sbuf, rbuf, type, op);
}
