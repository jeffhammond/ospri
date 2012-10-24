#include <pami.h>
#include "safemalloc.h"

#define PRINT_SUCCESS 0

#define PAMID_ASSERT(c,m) \
		do { \
			if (!(c)) { \
				printf(m" FAILED\n"); \
				fflush(stdout); \
                sleep(5);\
			    exit(50); \
			} \
			else if (PRINT_SUCCESS) { \
				printf(m" SUCCEEDED \n"); \
				fflush(stdout); \
                sleep(5);\
			} \
		} \
		while(0);

static int barrier(pami_geometry_t geometry, pami_context_t context)
{
	pami_result_t rc = PAMI_ERROR;

	pami_xfer_type_t xfer = PAMI_XFER_BARRIER;

	/* query the geometry */
	rc = PAMI_Geometry_algorithms_num( geometry, xfer, num_alg );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_num");

	size_t num_alg[2];
	pami_algorithm_t * safe_algs;
	pami_algorithm_t * fast_algs;
	pami_metadata_t  * safe_meta;
	pami_metadata_t  * fast_meta;

	safe_algs = (pami_algorithm_t *) PAMIU_Malloc( num_alg[0] * sizeof(pami_algorithm_t) );
	fast_algs = (pami_algorithm_t *) PAMIU_Malloc( num_alg[1] * sizeof(pami_algorithm_t) );
	safe_meta = (pami_metadata_t  *) PAMIU_Malloc( num_alg[0] * sizeof(pami_metadata_t)  );
	fast_meta = (pami_metadata_t  *) PAMIU_Malloc( num_alg[1] * sizeof(pami_metadata_t)  );
	rc = PAMI_Geometry_algorithms_query(geometry,
			xfer,
			safe_algs,
			safe_meta,
			num_alg[0],
			fast_algs,
			fast_meta,
			num_alg[1]);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Geometry_algorithms_query");

	size_t barrier_alg = 0; /* 0 is not necessarily the best one... */

	pami_xfer_t this;
	volatile int active = 0;

	this.cb_done   = cb_done;
	this.cookie    = (void*) &active;
	this.algorithm = safe_algs[barrier_alg]; /* safe algs should (must?) work */

	/* perform a barrier */
	active = 1;
	rc = PAMI_Collective( context, &this );
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Collective - barrier");

	while (active)
		rc = PAMI_Context_trylock_advancev( &context, 1, 1000 );

	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock_advancev - barrier");

	PAMIU_Free(safe_algs);
	PAMIU_Free(fast_algs);
	PAMIU_Free(safe_meta);
	PAMIU_Free(fast_meta);

	return PAMI_SUCCESS;
}
