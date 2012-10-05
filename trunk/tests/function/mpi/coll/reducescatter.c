#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

void reducescatterblock_only(FILE * output, MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    if ( comm_rank == 0 )
        fprintf(output, "============== REDUCESCATTERBLOCK ==============\n");

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/comm_size/sizeof(int);

    for (int c=1; c<max_count; c*=2)
    {
        int root = 0;

        int * sendbuf = (int *) safemalloc(c*comm_size*sizeof(int));
        int * recvbuf = (int *) safemalloc(c*sizeof(int));

        for (int i=0 ; i<c*comm_size; i++)
            sendbuf[i] = 0;

        for (int i=0 ; i<c; i++)
            sendbuf[comm_rank*c+i] = comm_rank;

        for (int i=0 ; i<c; i++)
            recvbuf[i] = 0;

        double t0 = MPI_Wtime();
        MPI_Reduce_scatter_block( sendbuf, recvbuf, c, MPI_INT, MPI_SUM, comm );
        double t1 = MPI_Wtime();
        double dt_redscat = t1-t0;

        int errors = 0;
        for (int i=0 ; i<c; i++)
            errors += (int) ( recvbuf[i] != comm_rank );

        if (errors>0)
        {
            fprintf(output, "MPI_Reduce_scatter_block had %d errors! \n", errors);
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "MPI_Reduce_scatter_block %d integers in %lf seconds (%lf MB/s) \n",
                   c*comm_size, dt_redscat, 1.0e-6*c*comm_size*sizeof(int)/dt_redscat );

        free(recvbuf);
        free(sendbuf);
    }

    return;
}
