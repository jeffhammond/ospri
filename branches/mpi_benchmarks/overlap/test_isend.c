#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <mpi.h>

int main(int argc, char * argv[])
{
    int rc, rc1, rc2;

    int requested = MPI_THREAD_SINGLE;
    int provided = -1;
    rc = MPI_Init_thread(&argc, &argv, requested, &provided);
    assert(rc==MPI_SUCCESS);
    assert(provided>=requested);

    int size = -1;
    rc = MPI_Comm_size(MPI_COMM_WORLD, &size);
    assert(rc==MPI_SUCCESS);
    assert(size>1);

    int rank = -1;
    rc = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    assert(rc==MPI_SUCCESS);

    int maxbytes = ( argc > 1 ? atoi(argv[1]) : 1024*1024 );

    int n = 0;
    for ( b=1; b<maxbytes; b*=2 ) n++;

    double * baseline = malloc(n*sizeof(double));


    for ( b=1; b<maxbytes; b*=2 )
    {
        if (rank==0)
        {
            double t0=0.0, t1=0.0;
            MPI_Request req;
            t0 = MPI_Wtime();
            rc1 = MPI_Isend(buf, b, MPI_BYTE, 1, tag, MPI_COMM_WORLD, &req);
            rc2 = MPI_Wait(&req, MPI_STATUS_IGNORE);
            t1 = MPI_Wtime();
            assert(rc1==MPI_SUCCESS && rc2=MPI_SUCCESS);

        }
        else if (rank==1)
        {
            rc = MPI_Recv(buf, b, MPI_BYTE, 0, tag, MPI_COMM_WORLD, MPI_STATUS_NULL);
            assert(rc==MPI_SUCCESS);
        }
        else
        {
        }

        rc = MPI_Barrier(MPI_COMM_WORLD);
        assert(rc==MPI_SUCCESS);
    }

    rc = MPI_Finalize();
    assert(rc==MPI_SUCCESS);

    return 0;
}
