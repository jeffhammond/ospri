#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

void alltoall_only(FILE * output, MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, world_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if ( comm_rank == 0 )
        fprintf(output, "============== ALLTOALL ==============\n");

    /* testing only integers because it really ~shouldn't~ matter */

    int max_count = max_mem/comm_size/sizeof(int);

    for (int c=1; c<max_count; c*=2)
    {
        int * in  = (int *) safemalloc(c*comm_size*sizeof(int));
        int * out = (int *) safemalloc(c*comm_size*sizeof(int));

        for (int i=0 ; i<(c*comm_size); i++)
            in[i]  = comm_rank;

        for (int i=0 ; i<(c*comm_size); i++)
            out[i] = -1;

        double t0 = MPI_Wtime();
        MPI_Alltoall( in, c, MPI_INT, out, c, MPI_INT, comm );
        double t1 = MPI_Wtime();
        double dt_a2a = t1-t0;

        int errors = 0;
        for (int i=0 ; i<(c*comm_size); i++)
            errors += (int) ( out[i] != i/c );

        if (errors>0)
        {
            fprintf(output, "%d: MPI_Alltoall had %d errors on rank %d! \n",
                   world_rank, errors, comm_rank);
            for (int i=0 ; i<(c*comm_size); i++)
                fprintf(output, "%d: buffer[%d] = %d (correct is %d) \n",
                       world_rank, i, out[i], i/c );
            exit(1);
        }

        if ( comm_rank == 0 )
            fprintf(output, "%d: MPI_Alltoall %d integers in %lf seconds (%lf MB/s) \n",
                   world_rank, c*comm_size, dt_a2a, 1.0e-6*c*comm_size*sizeof(int)/dt_a2a );

        free(out);
        free(in);
    }

    return;
}
