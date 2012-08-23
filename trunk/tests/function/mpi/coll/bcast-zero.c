#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int r, a;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&r);
    MPI_Bcast( &a, 0, MPI_INT, 0, MPI_COMM_WORLD );
    if (r==0) printf("MPI_Bcast of 0 integers succeeded.\n");
    MPI_Finalize();
    return 0;
}

