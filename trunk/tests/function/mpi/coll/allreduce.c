#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

void allreduce_only(FILE * output, MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    if ( comm_rank == 0 )
        fprintf(output, "============== ALLREDUCE ==============\n");

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/sizeof(int);

    for (int c=1; c<max_count; c*=2)
    {
        int root = 0;

        int * sendbuf = (int *) safemalloc(c*sizeof(int));
        int * recvbuf = (int *) safemalloc(c*sizeof(int));

        for (int i=0 ; i<c; i++)
            sendbuf[i] = 1;

        for (int i=0 ; i<c; i++)
            recvbuf[i] = 0;

        double t0 = MPI_Wtime();
        MPI_Allreduce( sendbuf, recvbuf, c, MPI_INT, MPI_SUM, comm );
        double t1 = MPI_Wtime();
        double dt_allred = t1-t0;

        int errors = 0;
        for (int i=0 ; i<c; i++)
            errors += (int) ( recvbuf[i] != comm_size );

        if (errors>0)
        {
            fprintf(output, "MPI_Allreduce had %d errors! \n", errors);
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "MPI_Allreduce %d integers in %lf seconds (%lf MB/s) \n",
                    c, dt_allred, 1.0e-6*c*sizeof(int)/dt_allred );

        free(recvbuf);
        free(sendbuf);
    }

    return;
}
