#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int provided;
    int size, rank;
    int namelen;
    char procname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init_thread( &argc, &argv, MPI_THREAD_SINGLE, &provided );
    assert( provided == MPI_THREAD_SINGLE );

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    printf( "Hello from %d of %d processors\n", rank, size );

    MPI_Barrier( MPI_COMM_WORLD );

    MPI_Get_processor_name( procname, &namelen );
    printf( "processor name = %s\n" , procname );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    MPI_Finalize();

    return 0;
}

