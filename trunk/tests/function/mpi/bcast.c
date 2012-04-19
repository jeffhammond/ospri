#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

void bcast_driver(MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/sizeof(int);

    for (int c=1; c<max_count; c++)
    {
        int root = 0;

        int value = 0;
        int * buffer = safemalloc(c*sizeof(int));

        if ( comm_rank == root )
            value = comm_size;
        else
            value = 0;

        for (int i=0 ; i< c; i++)
            buffer[i] = value;

        double t0 = MPI_Wtime();
        MPI_Bcast( buffer, c, MPI_INT, root, comm );
        double t1 = MPI_Wtime();

        int errors = 0;
        for (int i=0 ; i< c; i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            printf("MPI_Bcast had %d errors! \n", errors);
            exit(1)
        }

        if ( comm_rank == root )
            printf("MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c, t1-t0, 1.0e-6*c*sizeof(int)/(t1-t0) );

        free(buffer);
    }

    return;
}

