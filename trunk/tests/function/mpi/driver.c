#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

#define DEBUG YES

#define MPI_THREAD_STRING(level)  \
        ( level==MPI_THREAD_SERIALIZED ? "THREAD_SERIALIZED" : \
                ( level==MPI_THREAD_MULTIPLE ? "THREAD_MULTIPLE" : \
                        ( level==MPI_THREAD_FUNNELED ? "THREAD_FUNNELED" : \
                                ( level==MPI_THREAD_SINGLE ? "THREAD_SINGLE" : "WTF" ) ) ) )

int main(int argc, char *argv[])
{
    /*********************************************************************************
     *                            INITIALIZE MPI
     *********************************************************************************/

    int world_size = 0, world_rank = -1;

#if defined(USE_MPI_INIT)

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

#else

    int provided = -1, requested = -1;

#  if defined(USE_MPI_INIT_THREAD_MULTIPLE)
    requested = MPI_THREAD_MULTIPLE;
#  elif defined(USE_MPI_INIT_THREAD_SERIALIZED)
    requested = MPI_THREAD_SERIALIZED;
#  elif defined(USE_MPI_INIT_THREAD_FUNNELED)
    requested = MPI_THREAD_FUNNELED;
#  else
    requested = MPI_THREAD_SINGLE;
#  endif

    MPI_Init_thread( &argc, &argv, requested, &provided );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

    if (provided>requested)
    {
        if (world_rank==0) printf("MPI_Init_thread returned %s instead of %s, but this is okay. \n",
                                  MPI_THREAD_STRING(provided), MPI_THREAD_STRING(requested) );
    }
    if (provided<requested)
    {
        if (world_rank==0) printf("MPI_Init_thread returned %s instead of %s so the test will exit. \n",
                                  MPI_THREAD_STRING(provided), MPI_THREAD_STRING(requested) );
        exit(1);
    }

#endif

    MPI_Comm_size( MPI_COMM_WORLD, &world_size );
    if (world_rank==0) printf("MPI test program running on %d ranks. \n", world_size);

    /*********************************************************************************
     *                            SETUP MPI COMMUNICATORS
     *********************************************************************************/

    if (world_rank==0) printf("MPI_Barrier on MPI_COMM_WORLD 1 \n");
    MPI_Barrier( MPI_COMM_WORLD );

    if (world_rank==0) printf("MPI_Comm_dup of MPI_COMM_WORLD \n");
    MPI_Comm comm_world_dup;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm_world_dup);

    if (world_rank==0) printf("MPI_Barrier on comm_world_dup \n");
    MPI_Barrier( comm_world_dup );

    if (world_rank==0) printf("MPI_Comm_split of MPI_COMM_WORLD into odd-even \n");
    MPI_Comm comm_world_oddeven;
    MPI_Comm_split(MPI_COMM_WORLD, (int) (world_rank%2), world_rank, &comm_world_oddeven);

    if (world_rank==0) printf("MPI_Barrier on comm_world_oddeven \n");
    MPI_Barrier( comm_world_oddeven );

    if (world_rank==0) printf("MPI_Comm_split MPI_COMM_WORLD into (world-1) \n");
    MPI_Comm * comm_world_minus_ones = NULL;
    comm_world_minus_ones = (MPI_Comm *) safemalloc( world_size * sizeof(MPI_Comm) );
    for (int i=0; i<world_size; i++)
        MPI_Comm_split(MPI_COMM_WORLD, (int) (world_rank==i), world_rank, &comm_world_minus_ones[i]);

    if (world_rank==0) printf("MPI_Barrier on comm_world_minus_ones[] \n");
    for (int i=0; i<world_size; i++)
        MPI_Barrier( comm_world_minus_ones[i] );

    if (world_rank==0) printf("MPI_Comm_group of group_world from MPI_COMM_WORLD \n");
    MPI_Group group_world;
    MPI_Comm_group(MPI_COMM_WORLD, &group_world);

    int geomprog_size = (world_size==1) ? 1 : ceil(log2(world_size));

    int * geomprog_list = NULL;
    geomprog_list = (int *) safemalloc( geomprog_size * sizeof(int) );

    for (int i=0; i<geomprog_size; i++)
        geomprog_list[i] = pow(2,i)-1;

#ifdef DEBUG
    if (world_rank==0)
        for (int i=0; i<geomprog_size; i++)
            if (world_rank==0) printf("geomprog_list[%d] = %d \n", i, geomprog_list[i]);
#endif

    if (world_rank==0) printf("MPI_Group_incl of group_geomprog (geometric progression) from group_world \n");
    MPI_Group group_geomprog;
    MPI_Group_incl(group_world, geomprog_size, geomprog_list, &group_geomprog);

    if (world_rank==0) printf("MPI_Comm_create of comm_geomprog from group_geomprog on MPI_COMM_WORLD \n");
    MPI_Comm comm_geomprog;
    MPI_Comm_create(MPI_COMM_WORLD, group_geomprog, &comm_geomprog);

    if (world_rank==0) printf("MPI_Barrier on comm_geomprog \n");
    for (int i=0; i<geomprog_size; i++)
        if (geomprog_list[i]==world_rank)
            MPI_Barrier( comm_geomprog );

    if (world_rank==0) printf("MPI_Barrier on MPI_COMM_WORLD 2 \n");
    MPI_Barrier( MPI_COMM_WORLD );

    /*********************************************************************************
     *                            COLLECTIVES
     *********************************************************************************/

    //    MPI_Op builtin_types[19] = {    MPI_BYTE,
    //                                    MPI_FLOAT,
    //                                    MPI_DOUBLE,
    //                                    MPI_SHORT,
    //                                    MPI_INT,
    //                                    MPI_LONG,
    //                                    MPI_LONG_LONG_INT,
    //                                    MPI_UNSIGNED_SHORT,
    //                                    MPI_UNSIGNED
    //                                    MPI_UNSIGNED_LONG,
    //                                    MPI_UNSIGNED_LONG_LONG,
    //                                    MPI_INT8_T,
    //                                    MPI_INT16_T,
    //                                    MPI_INT32_T,
    //                                    MPI_INT64_T,
    //                                    MPI_UINT8_T,
    //                                    MPI_UINT16_T,
    //                                    MPI_UINT32_T,
    //                                    MPI_UINT64_T }
    //
    //    MPI_Op builtin_ops[15] = {  MPI_MAX,
    //                                MPI_MIN,
    //                                MPI_SUM,
    //                                MPI_PROD,
    //                                MPI_LAND,
    //                                MPI_BAND,
    //                                MPI_LOR,
    //                                MPI_BOR,
    //                                MPI_LXOR,
    //                                MPI_BXOR,
    //                                MPI_MINLOC,
    //                                MPI_MAXLOC,
    //                                MPI_REPLACE };

    bcast_only(MPI_COMM_WORLD, 1000000);
    bcast_vs_scatter_allgather(MPI_COMM_WORLD, 1000000);

    /*********************************************************************************
     *                            CLEAN UP AND FINALIZE
     *********************************************************************************/

    free(comm_world_minus_ones);
    free(geomprog_list);

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;
}
