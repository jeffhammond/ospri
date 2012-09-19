#ifndef INTERNALS_H
#define INTERNALS_H

#include "pamid.h"

void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

/*********** INTERNAL STATE ***********/

typedef struct {
	pami_xfer_type_t xfer;
	size_t num_alg[2];
	pami_algorithm_t * safe_algs;
	pami_algorithm_t * fast_algs;
	pami_metadata_t  * safe_meta;
	pami_metadata_t  * fast_meta;
} pamid_collective_state_t;

typedef struct {
	int local_blocking_context; /* synchronous:  for outbound/collective calls that block  */
	int local_offload_context;  /* asynchronous: for context post nonblocking operations   */
	int remote_put_context;     /* asynchronous: for PAMI_Rput and PAMI_Put 			   */
	int remote_acc_context;     /* asynchronous: for PAMI_Send doing accumulate            */
	int remote_rmw_context;     /* asynchronous: for PAMI_Send_immediate doing RMW         */
} pamid_context_roles_t;

typedef struct {
	pami_client_t pami_client;
	size_t world_rank;
	size_t world_size;
	size_t num_contexts;
	pami_context_t * pami_contexts;
	pamid_context_roles_t context_roles;
	pami_geometry_t world_geometry;
	pamid_collective_state_t world_barrier;
	pamid_collective_state_t world_bcast;
	pamid_collective_state_t world_allreduce;
} pamid_global_state_t;

extern pamid_global_state_t PAMID_INTERNAL_STATE;

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

/*********** INTERNAL DECLARATIONS ***********/

int PAMID_Progess_setup(int open, pami_context_t context);
int PAMID_Progess_teardown(int close, pami_context_t context);

int PAMID_Barrier_setup(pami_geometry_t geometry, pamid_collective_state_t * barrier);
int PAMID_Barrier_teardown(pamid_collective_state_t * barrier);
int PAMID_Barrier_doit(pamid_collective_state_t * barrier);

#endif /* INTERNALS_H */
