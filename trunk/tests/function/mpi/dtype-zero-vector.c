#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int r=-1;
    double a=37.0;
    MPI_Datatype t, u, v, w;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &r);

    MPI_Type_vector( 0, 0, 0, MPI_DOUBLE, &t);
    MPI_Type_vector( 1, 0, 0, MPI_DOUBLE, &u);
    MPI_Type_vector( 0, 1, 0, MPI_DOUBLE, &v);
    MPI_Type_vector( 1, 1, 0, MPI_DOUBLE, &w);

    MPI_Type_commit(&t);
    MPI_Type_commit(&u);
    MPI_Type_commit(&v);
    MPI_Type_commit(&w);

    MPI_Bcast( &a, 1, t, 0, MPI_COMM_WORLD );
    if (r==0) printf("MPI_Bcast of 0x0 datatype succeeded.\n");
    MPI_Bcast( &a, 1, u, 0, MPI_COMM_WORLD );
    if (r==0) printf("MPI_Bcast of 1x0 datatype succeeded.\n");
    MPI_Bcast( &a, 1, v, 0, MPI_COMM_WORLD );
    if (r==0) printf("MPI_Bcast of 0x1 datatype succeeded.\n");
    MPI_Bcast( &a, 1, w, 0, MPI_COMM_WORLD );
    if (r==0) printf("MPI_Bcast of 1x1 datatype succeeded.\n");

    MPI_Type_free(&t);
    MPI_Type_free(&u);
    MPI_Type_free(&v);
    MPI_Type_free(&w);

    MPI_Finalize();
    return 0;
}

