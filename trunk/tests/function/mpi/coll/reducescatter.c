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
            sendbuf[i] = 1;

        for (int i=0 ; i<c; i++)
            recvbuf[i] = 0;

        double t0 = MPI_Wtime();
        MPI_Reduce_scatter_block( sendbuf, recvbuf, c, MPI_INT, MPI_SUM, comm );
        double t1 = MPI_Wtime();
        double dt_redscat = t1-t0;

        int errors = 0;
        for (int i=0 ; i<c; i++)
            errors += (int) ( recvbuf[i] != comm_size );

        if (errors>0)
        {
            fprintf(output, "MPI_Bcast had %d errors! \n", errors);
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c*comm_size, dt_redscat, 1.0e-6*c*comm_size*sizeof(int)/dt_redscat );

        free(recvbuf);
        free(sendbuf);
    }

    return;
}

void reducescatterblock_vs_reduceandscatter(FILE * output, MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/comm_size/sizeof(int);

    for (int c=1; c<max_count; c*=2)
    {
        int root = 0;

        int value;
        if ( comm_rank == root )
            value = comm_size;
        else
            value = 0;

        double dt_bcast, dt_scalg, t0, t1;
        int errors;

        int * buffer = (int *) safemalloc(c*comm_size*sizeof(int));

        /*****************************************************/

        for (int i=0 ; i<(c*comm_size); i++)
            buffer[i] = value;

        t0 = MPI_Wtime();
        MPI_Bcast( buffer, c*comm_size, MPI_INT, root, comm );
        t1 = MPI_Wtime();
        dt_bcast = t1-t0;

        errors = 0;
        for (int i=0 ; i<(c*comm_size); i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            fprintf(output, "MPI_Bcast had %d errors on rank %d! \n", errors, comm_rank);
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c*comm_size, t1-t0, 1.0e-6*c*comm_size*sizeof(int)/(t1-t0) );

        /*****************************************************/

        int * temp = (int *) safemalloc(c*sizeof(int));

        for (int i=0 ; i<(c*comm_size); i++)
            buffer[i] = value;

        for (int i=0 ; i<c; i++)
            temp[i] = 0;

        t0 = MPI_Wtime();
        MPI_Scatter( buffer, c, MPI_INT, temp, c, MPI_INT, root, comm );
        MPI_Allgather( temp, c, MPI_INT, buffer, c, MPI_INT, comm );
        t1 = MPI_Wtime();
        dt_scalg = t1-t0;

        errors = 0;
        for (int i=0 ; i<(c*comm_size); i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            fprintf(output, "MPI_Scatter+MPI_Allgather had %d errors on rank %d! \n", errors, comm_rank);
            for (int i=0 ; i<(c*comm_size); i++)
                fprintf(output, "rank %d: buffer[%d] = %d (correct is %d) \n", comm_rank, i, buffer[i], comm_size );
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "MPI_Scatter+MPI_Allgather %d integers in %lf seconds (%lf MB/s) \n",
                   c*comm_size, t1-t0, 1.0e-6*c*comm_size*sizeof(int)/(t1-t0) );

        free(temp);

        /*****************************************************/

        free(buffer);
    }

    return;
}

void reducescatterblock_vs_allreduce(FILE * output, MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/comm_size/sizeof(int);

    for (int c=1; c<max_count; c*=2)
    {
        int root = 0;

        int value;
        if ( comm_rank == root )
            value = comm_size;
        else
            value = 0;

        double dt_bcast, dt_scalg, t0, t1;
        int errors;

        int * buffer = (int *) safemalloc(c*comm_size*sizeof(int));

        /*****************************************************/

        for (int i=0 ; i<(c*comm_size); i++)
            buffer[i] = value;

        t0 = MPI_Wtime();
        MPI_Bcast( buffer, c, MPI_INT, root, comm );
        t1 = MPI_Wtime();
        dt_bcast = t1-t0;

        errors = 0;
        for (int i=0 ; i<(c*comm_size); i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            fprintf(output, "MPI_Bcast had %d errors on rank %d! \n", errors, comm_rank);
            exit(1);
        }

        if ( comm_rank == root )
            fprintf(output, "MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c*comm_size, t1-t0, 1.0e-6*c*comm_size*sizeof(int)/(t1-t0) );

        /*****************************************************/

        {
            int * temp = (int *) safemalloc(c*sizeof(int));

            for (int i=0 ; i<(c*comm_size); i++)
                buffer[i] = value;

            for (int i=0 ; i<c; i++)
                temp[i] = 0;

            t0 = MPI_Wtime();
            MPI_Scatter( buffer, c, MPI_INT, temp,   c, MPI_INT, root, comm );
            MPI_Allgather( temp, c, MPI_INT, buffer, c, MPI_INT, comm );
            t1 = MPI_Wtime();
            dt_scalg = t1-t0;

            errors = 0;
            for (int i=0 ; i<(c*comm_size); i++)
                errors += (int) ( buffer[i] != comm_size );

            if (errors>0)
            {
                fprintf(output, "MPI_Scatter+MPI_Allgather had %d errors on rank %d! \n", errors, comm_rank);
                for (int i=0 ; i<(c*comm_size); i++)
                    fprintf(output, "rank %d: buffer[%d] = %d (correct is %d) \n", comm_rank, i, buffer[i], comm_size );
                exit(1);
            }

            if ( comm_rank == root )
                fprintf(output, "MPI_Scatter+MPI_Allgather %d integers in %lf seconds (%lf MB/s) \n",
                       c*comm_size, t1-t0, 1.0e-6*c*comm_size*sizeof(int)/(t1-t0) );

            free(temp);
        }

        /*****************************************************/

        free(buffer);
    }

    return;
}
