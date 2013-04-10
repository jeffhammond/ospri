#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

int main(int argc, char * argv[])
{
    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided!=MPI_THREAD_MULTIPLE) 
        MPI_Abort(MPI_COMM_WORLD, 1);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Finalize();

    return 0;
} 
