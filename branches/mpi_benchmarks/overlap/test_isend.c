#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

int main(int argc, char * argv[])
{
    int rc, rc1, rc2;

    int requested = MPI_THREAD_SINGLE, provided;
    MPI_Init_thread(&argc, &argv, requested, &provided);
    assert(provided>=requested);

    int r, rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int c, d, count = ( argc > 1 ? atoi(argv[1]) : 1024*1024 );

    double * rbuf = malloc( count * sizeof(double) );
    double * sbuf = malloc( count * sizeof(double) );
    assert(rbuf!=NULL && sbuf!=NULL);

    for (d=0; d<count; d++) sbuf[d] = (double)size; 

    for (c=1; c<count; c*=2)
    {
        double t0, t1;

        for (r=0; r<size; r++)
        {
            int tag = 1000;
            MPI_Request req[2];

            memset( rbuf, '\0', count*sizeof(double) );

            t0 = MPI_Wtime();

            if (rank==r)
                MPI_Irecv(rbuf, c, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, &req[0]);

            if (rank==0)
                MPI_Isend(sbuf, c, MPI_DOUBLE, r, tag, MPI_COMM_WORLD, &req[1]);

            if (rank==r)
		MPI_Wait( &req[0], MPI_STATUS_IGNORE );

            if (rank==0)
		MPI_Wait( &req[1], MPI_STATUS_IGNORE );

            t1 = MPI_Wtime();

            if (rank==r)
            {
                int errors = 0;
                for (d=0; d<c; d++) 
                    errors += (rbuf[d] != (double)size );
                if (errors>0) printf("There were %d errors! \n", errors);
            }

            if (rank==0)
                printf("%d->%d: bytes = %d bandwidth = %lf MB/s \n", 0, r, c*sizeof(double), 1e-6*c*sizeof(double)/(t1-t0) );

        }
        if (rank==0)
        {
            t0 = MPI_Wtime();
            memcpy( rbuf, sbuf, c*sizeof(double) );
            t1 = MPI_Wtime();
            printf("memcpy: bytes = %d bandwidth = %lf MB/s \n", c*sizeof(double), 1e-6*c*sizeof(double)/(t1-t0) );
        }
        fflush(stdout);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
