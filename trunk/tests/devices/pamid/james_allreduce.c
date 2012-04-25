#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

pami_client_t client;
pami_context_t * contexts = NULL;

typedef struct MPI_Comm
{
    int is_world;
    pami_geometry_t geom;
    pami_algorithm_t * safe_barrier_algs;
    pami_algorithm_t * safe_allreduce_algs;
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
        case MPI_UNSIGNED_INT:
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
            pami_op = PAMI_DATA_MAX;
            break;
        case MPI_MIN:
            pami_op = PAMI_DATA_MIN;
            break;
        case MPI_SUM:
            pami_op = PAMI_DATA_SUM;
            break;
        case MPI_PROD:
            pami_op = PAMI_DATA_PROD;
            break;
        case MPI_BAND:
            pami_op = PAMI_DATA_BAND;
            break;
        case MPI_BOR:
            pami_op = PAMI_DATA_BOR;
            break;
        case MPI_BXOR:
            pami_op = PAMI_DATA_BXOR;
            break;
        case MPI_LAND:
            pami_op = PAMI_DATA_LAND;
            break;
        case MPI_LOR:
            pami_op = PAMI_DATA_LOR;
            break;
        case MPI_LXOR:
            pami_op = PAMI_DATA_LXOR;
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

int MPI_Init(int argc, char* argv[])
{
    pami_result_t result = PAMI_ERROR;
    char * clientname = "MPI";
    result = PAMI_Client_create( clientname, &client, NULL, 0 );
    if (result!=PAMI_SUCCESS) exit(1); 
 
    pami_configuration_t config;
    config.name = PAMI_CLIENT_NUM_CONTEXTS;
    result = PAMI_Client_query( client, &config, 1);
    if (result!=PAMI_SUCCESS) exit(1); 
    num_contexts = config.value.intval;
 
    /* initialize the contexts */
    contexts = (pami_context_t *) safemalloc( num_contexts * sizeof(pami_context_t) );
 
    result = PAMI_Context_createv( client, &config, 0, contexts, num_contexts );
    if (result!=PAMI_SUCCESS) exit(1); 
 
    /* setup the world geometry */
    result = PAMI_Geometry_world( client, MPI_COMM_WORLD->world_geometry );
    if (result!=PAMI_SUCCESS) exit(1); 

    MPI_COMM_WORLD.is_world = 1;
 
    pami_xfer_type_t barrier_xfer = PAMI_XFER_BARRIER;
    size_t num_barrier_alg[2];
 
    /* barrier algs */
    result = PAMI_Geometry_algorithms_num( world_geometry, barrier_xfer, num_barrier_alg );
    if (result!=PAMI_SUCCESS) exit(1); 
 
       MPI_COMM_WORLD->safe_barrier_algs = (pami_algorithm_t *) safemalloc( num_barrier_alg[0] * sizeof(pami_algorithm_t) );
    pami_metadata_t  * safe_barrier_meta = (pami_metadata_t  *) safemalloc( num_barrier_alg[0] * sizeof(pami_metadata_t)  );
    pami_algorithm_t * fast_barrier_algs = (pami_algorithm_t *) safemalloc( num_barrier_alg[1] * sizeof(pami_algorithm_t) );
    pami_metadata_t  * fast_barrier_meta = (pami_metadata_t  *) safemalloc( num_barrier_alg[1] * sizeof(pami_metadata_t)  );
    result = PAMI_Geometry_algorithms_query( world_geometry, barrier_xfer,
                                             safe_barrier_algs, safe_barrier_meta, num_barrier_alg[0],
                                             fast_barrier_algs, fast_barrier_meta, num_barrier_alg[1] );
    if (result!=PAMI_SUCCESS) exit(1); 
 
    /* allreduce algs */
    pami_xfer_type_t allreduce_xfer = PAMI_XFER_ALLREDUCE;
    size_t num_allreduce_alg[2];
 
    result = PAMI_Geometry_algorithms_num( world_geometry, allreduce_xfer, num_allreduce_alg );
    if (result!=PAMI_SUCCESS) exit(1); 
 
       MPI_COMM_WORLD->safe_allreduce_algs = (pami_algorithm_t *) safemalloc( num_allreduce_alg[0] * sizeof(pami_algorithm_t) );
    pami_metadata_t  * safe_allreduce_meta = (pami_metadata_t  *) safemalloc( num_allreduce_alg[0] * sizeof(pami_metadata_t)  );
    pami_algorithm_t * fast_allreduce_algs = (pami_algorithm_t *) safemalloc( num_allreduce_alg[1] * sizeof(pami_algorithm_t) );
    pami_metadata_t  * fast_allreduce_meta = (pami_metadata_t  *) safemalloc( num_allreduce_alg[1] * sizeof(pami_metadata_t)  );
    result = PAMI_Geometry_algorithms_query( world_geometry, allreduce_xfer,
                                             safe_allreduce_algs, safe_allreduce_meta, num_allreduce_alg[0],
                                             fast_allreduce_algs, fast_allreduce_meta, num_allreduce_alg[1] );
    if (result!=PAMI_SUCCESS) exit(1); 
 
    return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
    pami_result_t result = PAMI_ERROR;
    result = PAMI_Context_destroyv( contexts, num_contexts );
    if (result!=PAMI_SUCCESS) exit(1); 
   
    free(contexts);
   
    result = PAMI_Client_destroy( &client );
    if (result!=PAMI_SUCCESS) exit(1); 

    return MPI_SUCCESS;
}

int MPI_Comm_rank(MPI_Comm comm, int * rank)
{
    if (comm.is_world!=1) exit(1); 
    pami_configuration_t config;
    config.name = PAMI_CLIENT_TASK_ID;
    result = PAMI_Client_query(client, &config, 1);
    if (result!=PAMI_SUCCESS) exit(1); 

    (*rank) = config.value.intval;

    return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int * size)
{
    if (comm.is_world!=1) exit(1); 
    pami_configuration_t config;
    config.name = PAMI_CLIENT_NUM_TASKS;
    result = PAMI_Client_query(client, &config, 1);
    if (result!=PAMI_SUCCESS) exit(1); 
    (*size) = config.value.intval;
    return MPI_SUCCESS;
}

const size_t barrier_alg_num = 0;

int MPI_Barrier(MPI_Comm comm)
{
    volatile int active = 0;
    pami_xfer_t barrier;
    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) &active;
    barrier.algorithm = safe_barrier_algs[barrier_alg_num];

    active = 1;
    result = PAMI_Collective( contexts[0], &barrier );
    if (result!=PAMI_SUCCESS) exit(1); 
    while (active)
    {
        result = PAMI_Context_advance(contexts[0], 1);
        if (result!=PAMI_SUCCESS) exit(1);
    }

    return MPI_SUCCESS;
}

const size_t allreduce_alg_num = 0;

int MPI_Allreduce(void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    pami_type_t pami_dt;
    pami_data_function pami_op;
    MPI_to_PAMI_dt(datatype, &pami_dt);
    MPI_to_PAMI_op(op, &pami_op);

    volatile int active = 0;
    pami_xfer_t allreduce;
    allreduce.cb_done   = cb_done;
    allreduce.cookie    = (void*) &active;
    allreduce.algorithm = safe_allreduce_algs[allreduce_alg_num];
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
        if (result!=PAMI_SUCCESS) exit(1);
    }

    return MPI_SUCCESS;
}

#define MAXLEN (1024*1024)
#define REPEAT 100

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

