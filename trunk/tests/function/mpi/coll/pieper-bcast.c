#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

#define MPI_THREAD_STRING(level)  \
        ( level==MPI_THREAD_SERIALIZED ? "THREAD_SERIALIZED" : \
                ( level==MPI_THREAD_MULTIPLE ? "THREAD_MULTIPLE" : \
                        ( level==MPI_THREAD_FUNNELED ? "THREAD_FUNNELED" : \
                                ( level==MPI_THREAD_SINGLE ? "THREAD_SINGLE" : "WTF" ) ) ) )

static void bcast_only(FILE * output, MPI_Comm comm)
{
    int comm_rank = -1, world_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if ( comm_rank == 0 )
        fprintf(output, "============== BCAST ==============\n");

    int max_count = 1024*1024*1024;

    for (int c=1; c<max_count; c*=2)
    {
        int root = 0;

        double value = 0;
        double * buffer = (double *) safemalloc(c*sizeof(double));

        if ( comm_rank == root )
            value = (double)comm_size;
        else
            value = 0.0;

        for (int i=0 ; i< c; i++)
            buffer[i] = value;

        double t0 = MPI_Wtime();
        MPI_Bcast( buffer, c, MPI_INT, root, comm );
        double t1 = MPI_Wtime();
        double dt_bcast = t1-t0;

        int errors = 0;
        for (int i=0 ; i< c; i++)
            errors += (int) ( buffer[i] != (double)comm_size );

        if (errors>0)
        {
            fprintf(output, "%d: MPI_Bcast had %d errors on rank %d! \n",
                   world_rank, errors, comm_rank);
            for (int i=0 ; i<c; i++)
                fprintf(output, "%d: buffer[%d] = %lf (correct is %lf) \n",
                       world_rank, i, buffer[i], (double)comm_size );
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "%d: MPI_Bcast %d doubles in %lf seconds (%lf MB/s) \n",
                   world_rank, c, dt_bcast, 1.0e-6*c*sizeof(double)/dt_bcast );

        free(buffer);
    }

    return;
}

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

    double t0 = MPI_Wtime();

    int is_init = 0;
    MPI_Initialized(&is_init);
    if (world_rank==0) printf("MPI %s initialized. \n", (is_init==1 ? "was" : "was not") );

    MPI_Query_thread(&provided);
    if (world_rank==0) printf("MPI thread support is %s. \n", MPI_THREAD_STRING(provided) );

    MPI_Comm_size( MPI_COMM_WORLD, &world_size );
    if (world_rank==0) printf("MPI test program running on %d ranks. \n", world_size);

    /*********************************************************************************
     *                            COLLECTIVES
     *********************************************************************************/

    MPI_Comm test_comm;

    test_comm = MPI_COMM_WORLD;

    MPI_Barrier( MPI_COMM_WORLD );

    if (world_rank==0)
    	printf("############## %s ##############\n", "MPI_COMM_WORLD" );

	bcast_only(stdout, test_comm);

    fflush(stdout);
    MPI_Barrier( MPI_COMM_WORLD );

    double t1 = MPI_Wtime();
    double dt = t1-t0;
    if (world_rank==0)
       printf("TEST FINISHED SUCCESSFULLY IN %lf SECONDS \n", dt);
    fflush(stdout);

    MPI_Finalize();

    return 0;
}
