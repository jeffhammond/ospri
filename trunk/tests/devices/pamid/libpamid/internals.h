#ifndef INTERNALS_H
#define INTERNALS_H

#include "pamid.h"

/*********** PAMI CALLBACKS ***********/

void cb_done(void *ctxt, void * clientdata, pami_result_t err);
void cb_local_done (void *ctxt, void * clientdata, pami_result_t err);
void cb_remote_done (void *ctxt, void * clientdata, pami_result_t err);

/*********** INTERNAL STATE ***********/

extern int pamid_progress_active;
extern pthread_t PAMID_Progress_thread;

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
    int remote_put_context;     /* asynchronous: for PAMI_Rput and PAMI_Put                */
    int remote_get_context;     /* asynchronous: for PAMI_Rput and PAMI_Put                */
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
    pamid_collective_state_t world_sync;
    pamid_collective_state_t world_bcast;
    pamid_collective_state_t world_allreduce;
    pamid_collective_state_t world_allgather;
} pamid_global_state_t;

extern pamid_global_state_t PAMID_INTERNAL_STATE;

/*********** MACROS ***********/

#define PRINT_SUCCESS 1

#define PAMID_ASSERT(c,m) \
        do { \
            if (!(c)) { \
                fprintf(stderr,m" FAILED\n"); \
                fflush(stderr); \
                abort(); \
            } \
            else if (PRINT_SUCCESS) { \
                fprintf(stderr,m" SUCCEEDED \n"); \
                fflush(stderr); \
            } \
        } \
        while(0);

/*********** INTERNAL DECLARATIONS ***********/

void * PAMID_Progress_function(void * dummy);
int PAMID_Progress_poke(void);

int PAMID_Progess_setup(int open, pami_context_t context);
int PAMID_Progess_teardown(int close, pami_context_t context);

int PAMID_Barrier_setup(pami_geometry_t geometry, pamid_collective_state_t * barrier);
int PAMID_Barrier_teardown(pamid_collective_state_t * barrier);
int PAMID_Barrier_doit(pamid_collective_state_t * barrier);

int PAMID_Broadcast_setup(pami_geometry_t geometry, pamid_collective_state_t * broadcast);
int PAMID_Broadcast_teardown(pamid_collective_state_t * broadcast);
int PAMID_Broadcast_doit(pamid_collective_state_t * broadcast, int root, size_t num_bytes, void * buffer);

int PAMID_Allgather_setup(pami_geometry_t geometry, pamid_collective_state_t * allgather);
int PAMID_Allgather_teardown(pamid_collective_state_t * allgather);
int PAMID_Allgather_doit(pamid_collective_state_t * allgather, size_t num_bytes, void * sbuf, void * rbuf);

int PAMID_Allreduce_setup(pami_geometry_t geometry, pamid_collective_state_t * allreduce);
int PAMID_Allreduce_teardown(pamid_collective_state_t * allreduce);
int PAMID_Allreduce_doit(pamid_collective_state_t * allreduce, size_t count, void * sbuf, void * rbuf, pami_type_t type, pami_data_function op);

#endif /* INTERNALS_H */
