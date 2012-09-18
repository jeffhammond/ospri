#ifndef INTERNALS_H
#define INTERNALS_H

#include "pamid.h"

/*********** INTERNAL STATE ***********/

typedef struct {
	pami_client_t pami_client;
	size_t world_rank;
	size_t world_size;
	size_t num_contexts;
	pami_context_t * pami_contexts;
} global_state_t;

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
