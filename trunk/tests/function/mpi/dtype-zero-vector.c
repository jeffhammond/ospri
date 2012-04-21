#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int r, a;
    MPI_Datatype t;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&r);
    MPI_Type_vector( 0, 0, 0, MPI_INT, &t);
    MPI_Type_commit(&t);
    MPI_Bcast( &a, 1, t, 0, MPI_COMM_WORLD );
    MPI_Type_free(&t);
    if (r==0) printf("MPI_Bcast of 0x0 datatype succeeded.\n");
    MPI_Finalize();
    return 0;
}

