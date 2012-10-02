#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"
#include "driver.h"

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
    int provided = -1;

#if defined(USE_MPI_INIT)

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

#else

    int requested = -1;

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

    int is_init = 0;
    MPI_Initialized(&is_init);
    if (world_rank==0) printf("MPI %s initialized. \n", (is_init==1 ? "was" : "was not") );

    MPI_Query_thread(&provided);
    if (world_rank==0) printf("MPI thread support is %s. \n", MPI_THREAD_STRING(provided) );

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

    if (world_rank==0) printf("MPI_Comm_split MPI_COMM_WORLD into (world-1) \n");
    MPI_Comm * comm_world_minus_ones = NULL;
    comm_world_minus_ones = (MPI_Comm *) safemalloc( world_size * sizeof(MPI_Comm) );
    for (int i=0; i<world_size; i++)
        MPI_Comm_split(MPI_COMM_WORLD, (int) (world_rank==i), world_rank, &comm_world_minus_ones[i]);

    if (world_rank==0) printf("MPI_Barrier on comm_world_minus_ones[] \n");
    for (int i=0; i<world_size; i++)
        MPI_Barrier( comm_world_minus_ones[i] );

    for (int i=0; i<world_size; i++)
        MPI_Comm_free(&comm_world_minus_ones[i]);

    MPI_Comm_free(&comm_world_dup);

    free(comm_world_minus_ones);

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;
}
