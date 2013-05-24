#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

//#include "safemalloc.h"
int posix_memalign(void **memptr, size_t alignment, size_t size);
static void * safemalloc(size_t n) 
{
    void * ptr = NULL;
    int rc = posix_memalign(&ptr , 128, n);
    if (ptr==NULL || n<0) {
        fprintf( stderr , "%d bytes could not be allocated \n" , n );
        exit(n);
    }
    return ptr;
}

#define RESULT_CHECK(result) \
if (result!=PAMI_SUCCESS) { printf("result!=PAMI_SUCCESS\n"); fflush(NULL); exit(1); }

pami_client_t client;
size_t num_contexts;
pami_context_t * contexts = NULL;
size_t task_id;
size_t num_tasks;

const size_t barrier_alg_num = 0;
const size_t allreduce_alg_num = 0;

typedef struct MPI_Comm
{
    int                is_world;
    pami_geometry_t    geometry;

    pami_xfer_type_t   barrier_xfer;
    size_t             num_barrier_alg[2];
    pami_algorithm_t * safe_barrier_algs;
    pami_metadata_t  * safe_barrier_meta;
    pami_algorithm_t * fast_barrier_algs;
    pami_metadata_t  * fast_barrier_meta;

    pami_xfer_type_t   allreduce_xfer;
    size_t             num_allreduce_alg[2];
    pami_algorithm_t * safe_allreduce_algs;
    pami_metadata_t  * safe_allreduce_meta;
    pami_algorithm_t * fast_allreduce_algs;
    pami_metadata_t  * fast_allreduce_meta;
} 
MPI_Comm;

MPI_Comm MPI_COMM_WORLD;

#define MPI_SUCCESS 0

enum MPI_Op 
{ 
    MPI_MAX, MPI_MIN, 
    MPI_SUM, MPI_PROD,
    MPI_LAND, MPI_LOR, MPI_LXOR,
    MPI_BAND, MPI_BOR, MPI_BXOR,
    MPI_MINLOC, MPI_MAXLOC,
    MPI_REPLACE,
};

enum MPI_Datatype
{
    MPI_FLOAT, MPI_DOUBLE, MPI_BYTE,
    MPI_SHORT, MPI_INT, MPI_LONG, MPI_LONG_LONG,
    MPI_UNSIGNED_SHORT, MPI_UNSIGNED, MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG_LONG
};

static inline void MPI_to_PAMI_dt(int mpi_dt, pami_type_t * pami_dt)
{
    switch (mpi_dt) 
    {
        case MPI_FLOAT:
            pami_dt = PAMI_TYPE_FLOAT;
            break;
        case MPI_DOUBLE:
            pami_dt = PAMI_TYPE_DOUBLE;
            break;
        case MPI_BYTE:
            pami_dt = PAMI_TYPE_BYTE;
            break;
        case MPI_SHORT:
            pami_dt = PAMI_TYPE_SIGNED_SHORT;
            break;
        case MPI_INT:
            pami_dt = PAMI_TYPE_SIGNED_INT;
            break;
        case MPI_LONG:
            pami_dt = PAMI_TYPE_SIGNED_LONG;
            break;
        case MPI_LONG_LONG:
            pami_dt = PAMI_TYPE_SIGNED_LONG_LONG;
            break;
        case MPI_UNSIGNED_SHORT:
            pami_dt = PAMI_TYPE_UNSIGNED_SHORT;
            break;
        case MPI_UNSIGNED:
            pami_dt = PAMI_TYPE_UNSIGNED_INT;
            break;
        case MPI_UNSIGNED_LONG:
            pami_dt = PAMI_TYPE_UNSIGNED_LONG;
            break;
        case MPI_UNSIGNED_LONG_LONG:
            pami_dt = PAMI_TYPE_UNSIGNED_LONG_LONG;
            break;
        default:
            printf("MPI_to_PAMI_dt: unsupported or invalid MPI_Datatype\n");
            exit(1);
    }
    return;
}

static inline void MPI_to_PAMI_op(int mpi_op, pami_data_function * pami_op)
{
    switch (mpi_op) 
    {
        case MPI_MAX:
            *pami_op = PAMI_DATA_MAX;
            break;
        case MPI_MIN:
            *pami_op = PAMI_DATA_MIN;
            break;
        case MPI_SUM:
            *pami_op = PAMI_DATA_SUM;
            break;
        case MPI_PROD:
            *pami_op = PAMI_DATA_PROD;
            break;
        case MPI_BAND:
            *pami_op = PAMI_DATA_BAND;
            break;
        case MPI_BOR:
            *pami_op = PAMI_DATA_BOR;
            break;
        case MPI_BXOR:
            *pami_op = PAMI_DATA_BXOR;
            break;
        case MPI_LAND:
            *pami_op = PAMI_DATA_LAND;
            break;
        case MPI_LOR:
            *pami_op = PAMI_DATA_LOR;
            break;
        case MPI_LXOR:
            *pami_op = PAMI_DATA_LXOR;
            break;
        default:
            printf("MPI_to_PAMI_op: unsupported or invalid MPI_Op\n");
            exit(1);
    }
    return;
}

void cb_done (void *context , void * clientdata, pami_result_t error)
{
  int * active = (int *) clientdata;
  (*active)--;
}

int MPI_Init(int * argc, char ** argv[])
{
    pami_result_t result = PAMI_ERROR;
    char * clientname = "MPI";
    result = PAMI_Client_create( clientname, &client, NULL, 0 );
    RESULT_CHECK(result); 
 
    pami_configuration_t config;

    config.name = PAMI_CLIENT_TASK_ID;
    result = PAMI_Client_query(client, &config, 1);
    RESULT_CHECK(result); 
    task_id = config.value.intval;

    config.name = PAMI_CLIENT_NUM_TASKS;
    result = PAMI_Client_query(client, &config, 1);
    RESULT_CHECK(result); 
    num_tasks = config.value.intval;

    config.name = PAMI_CLIENT_NUM_CONTEXTS;
    result = PAMI_Client_query( client, &config, 1);
    RESULT_CHECK(result); 
    num_contexts = config.value.intval;
 
    printf("MPI_Init: rank %ld of %ld, %ld contexts available \n", task_id, num_tasks, num_contexts);

    /* initialize the contexts */
    contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );
 
    result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts );
    RESULT_CHECK(result); 
 
    /* setup the world geometry */
    result = PAMI_Geometry_world( client, MPI_COMM_WORLD.geometry );
    RESULT_CHECK(result); 

    MPI_COMM_WORLD.is_world = 1;
    MPI_COMM_WORLD.barrier_xfer   = PAMI_XFER_BARRIER;
 
    /* barrier algs */
    result = PAMI_Geometry_algorithms_num( MPI_COMM_WORLD.geometry, MPI_COMM_WORLD.barrier_xfer, MPI_COMM_WORLD.num_barrier_alg );
    RESULT_CHECK(result); 
 
    MPI_COMM_WORLD.safe_barrier_algs = (pami_algorithm_t *) safemalloc( MPI_COMM_WORLD.num_barrier_alg[0] * sizeof(pami_algorithm_t) );
    MPI_COMM_WORLD.safe_barrier_meta = (pami_metadata_t  *) safemalloc( MPI_COMM_WORLD.num_barrier_alg[0] * sizeof(pami_metadata_t)  );
    MPI_COMM_WORLD.fast_barrier_algs = (pami_algorithm_t *) safemalloc( MPI_COMM_WORLD.num_barrier_alg[1] * sizeof(pami_algorithm_t) );
    MPI_COMM_WORLD.fast_barrier_meta = (pami_metadata_t  *) safemalloc( MPI_COMM_WORLD.num_barrier_alg[1] * sizeof(pami_metadata_t)  );
    result = PAMI_Geometry_algorithms_query( MPI_COMM_WORLD.geometry, MPI_COMM_WORLD.barrier_xfer,
                                             MPI_COMM_WORLD.safe_barrier_algs, MPI_COMM_WORLD.safe_barrier_meta, MPI_COMM_WORLD.num_barrier_alg[0],
                                             MPI_COMM_WORLD.fast_barrier_algs, MPI_COMM_WORLD.fast_barrier_meta, MPI_COMM_WORLD.num_barrier_alg[1] );
    RESULT_CHECK(result); 
 
    /* allreduce algs */
    MPI_COMM_WORLD.allreduce_xfer = PAMI_XFER_ALLREDUCE;
 
    result = PAMI_Geometry_algorithms_num( MPI_COMM_WORLD.geometry, MPI_COMM_WORLD.allreduce_xfer, MPI_COMM_WORLD.num_allreduce_alg );
    RESULT_CHECK(result); 
 
    MPI_COMM_WORLD.safe_allreduce_algs = (pami_algorithm_t *) safemalloc( MPI_COMM_WORLD.num_allreduce_alg[0] * sizeof(pami_algorithm_t) );
    MPI_COMM_WORLD.safe_allreduce_meta = (pami_metadata_t  *) safemalloc( MPI_COMM_WORLD.num_allreduce_alg[0] * sizeof(pami_metadata_t)  );
    MPI_COMM_WORLD.fast_allreduce_algs = (pami_algorithm_t *) safemalloc( MPI_COMM_WORLD.num_allreduce_alg[1] * sizeof(pami_algorithm_t) );
    MPI_COMM_WORLD.fast_allreduce_meta = (pami_metadata_t  *) safemalloc( MPI_COMM_WORLD.num_allreduce_alg[1] * sizeof(pami_metadata_t)  );
    result = PAMI_Geometry_algorithms_query( MPI_COMM_WORLD.geometry, MPI_COMM_WORLD.allreduce_xfer,
                                             MPI_COMM_WORLD.safe_allreduce_algs, MPI_COMM_WORLD.safe_allreduce_meta, MPI_COMM_WORLD.num_allreduce_alg[0],
                                             MPI_COMM_WORLD.fast_allreduce_algs, MPI_COMM_WORLD.fast_allreduce_meta, MPI_COMM_WORLD.num_allreduce_alg[1] );
    RESULT_CHECK(result); 
 
    return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
    pami_result_t result = PAMI_ERROR;
    result = PAMI_Context_destroyv( contexts, num_contexts );
    RESULT_CHECK(result); 
   
    free(contexts);
   
    result = PAMI_Client_destroy( &client );
    RESULT_CHECK(result); 

    return MPI_SUCCESS;
}

double MPI_Wtime(void)
{
    return PAMI_Wtime(client);
}

int MPI_Comm_rank(MPI_Comm comm, int * rank)
{
    if (comm.is_world!=1) exit(1); 
    *rank = (int)task_id;
    return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int * size)
{
    if (comm.is_world!=1) exit(1); 
    *size = (int)num_tasks;
    return MPI_SUCCESS;
}

int MPI_Barrier(MPI_Comm comm)
{
    pami_result_t result = PAMI_ERROR;

    volatile int active = 0;
    pami_xfer_t barrier;
    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) &active;
    barrier.algorithm = comm.safe_barrier_algs[barrier_alg_num];

    active = 1;
    result = PAMI_Collective( contexts[0], &barrier );
    RESULT_CHECK(result); 
    while (active)
    {
        result = PAMI_Context_advance(contexts[0], 1);
        RESULT_CHECK(result);
    }

    return MPI_SUCCESS;
}

int MPI_Allreduce(void * sendbuf, void * recvbuf, int count, int datatype, int op, MPI_Comm comm)
{
    pami_result_t result = PAMI_ERROR;

    pami_type_t pami_dt = PAMI_TYPE_BYTE;
    pami_data_function pami_op;
    MPI_to_PAMI_dt(datatype, &pami_dt);
    MPI_to_PAMI_op(op, &pami_op);

    volatile int active = 0;
    pami_xfer_t allreduce;
    allreduce.cb_done   = cb_done;
    allreduce.cookie    = (void*) &active;
    allreduce.algorithm = comm.safe_allreduce_algs[allreduce_alg_num];
    allreduce.cmd.xfer_allreduce.op         = pami_op;
    allreduce.cmd.xfer_allreduce.sndbuf     = sendbuf;
    allreduce.cmd.xfer_allreduce.stype      = pami_dt;
    allreduce.cmd.xfer_allreduce.stypecount = count;
    allreduce.cmd.xfer_allreduce.rcvbuf     = recvbuf;
    allreduce.cmd.xfer_allreduce.rtype      = pami_dt;
    allreduce.cmd.xfer_allreduce.rtypecount = count;

    active = 1;
    result = PAMI_Collective( contexts[0], &allreduce );
    while (active)
    {
        result = PAMI_Context_advance(contexts[0], 1);
        RESULT_CHECK(result);
    }

    return MPI_SUCCESS;
}

#define MAXLEN (32*1024)
#define REPEAT 20

int main(int argc, char *argv[])
{
    double x[REPEAT][MAXLEN], r[REPEAT][MAXLEN];
    double secs;
    int rank;
    int i, j, len;
   
    MPI_Init(&argc, &argv);
   
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   
    for(i=0; i<REPEAT; i++)
        for(j=0; j<MAXLEN; j++)
            x[i][j] = 1.7*rank + j;
   
    for(len=1; len<=MAXLEN; len*=2)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        secs = -MPI_Wtime();
        for(i=0; i<REPEAT; i++)
            MPI_Allreduce((void *)x[i], (void *)r[i], len, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        secs += MPI_Wtime();
       
        if(rank==0)
            printf("%i\t%g\n", len, 1e6*secs/REPEAT);
    }
   
    MPI_Finalize();
    return MPI_SUCCESS;
}

