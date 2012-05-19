#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

void alltoall_only(MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    if ( comm_rank == 0 )
        printf("============== ALLTOALL ==============\n");

    /* testing only integers because it really ~shouldn't~ matter */

    int max_count = max_mem/sizeof(int);

    for (int c=1; c<max_count; c*=2)
        if (c%comm_size==0)
        {
            int * in  = (int *) safemalloc(c*sizeof(int));
            int * out = (int *) safemalloc(c*sizeof(int));

            for (int i=0 ; i<c; i++)
                in[i]  = comm_rank;

            for (int i=0 ; i<c; i++)
                out[i] = -1;

            double t0 = MPI_Wtime();
            MPI_Alltoall( in, c/comm_size, MPI_INT, out, c/comm_size, MPI_INT, comm );
            double t1 = MPI_Wtime();
            double dt_a2a = t1-t0;

            int errors = 0;
            for (int i=0 ; i<c; i++)
                errors += (int) ( out[i] != i/(c/comm_size) );

            if (errors>0)
            {
                printf("MPI_Alltoall had %d errors on rank %d! \n", errors, comm_rank);
                for (int i=0 ; i<c; i++)
                    printf("rank %d: buffer[%d] = %d (correct is %d) \n", comm_rank,
                           i, out[i], i/(c/comm_size) );
                exit(1);
            }

            if ( comm_rank == 0 )
                printf("MPI_Alltoall %d integers in %lf seconds (%lf MB/s) \n",
                       c, dt_a2a, 1.0e-6*c*sizeof(int)/dt_a2a );

            free(out);
            free(in);
        }

    return;
}
