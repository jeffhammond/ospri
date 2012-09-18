#ifndef INTERNALS_H
#define INTERNALS_H

#include "pamid.h"

/*********** INTERNAL STATE ***********/

typedef struct {
	pami_xfer_type_t xfer = PAMI_XFER_BARRIER;
	size_t num_alg[2];
	pami_algorithm_t * safe_algs;
	pami_algorithm_t * fast_algs;
	pami_metadata_t  * safe_meta;
	pami_metadata_t  * fast_meta;
} pamid_barrier_state_t;

typedef struct {
	pami_client_t pami_client;
	size_t world_rank;
	size_t world_size;
	size_t num_contexts;
	pami_context_t * pami_contexts;
	pami_geometry_t world_geometry;
	pamid_barrier_state_t world_barrier;
} pamid_global_state_t;

extern global_state_t PAMID_INTERNAL_STATE;

/*********** MACROS ***********/

#define PRINT_SUCCESS 0

#define PAMID_ASSERT(c,m) \
		do { \
			if (!(c)) { \
				printf(m" FAILED\n"); \
				fflush(stdout); \
			} \
			else if (PRINT_SUCCESS) { \
				printf(m" SUCCEEDED \n"); \
				fflush(stdout); \
			} \
			exit(1); \
		} \
		while(0);


#endif /* INTERNALS_H */
