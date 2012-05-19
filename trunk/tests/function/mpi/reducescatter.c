#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"

void reducescatter_only(MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    if ( comm_rank == 0 )
        printf("============== REDUCESCATTER ==============\n");

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/sizeof(int);

    for (int c=1; c<max_count; c*=2)
    {
        int root = 0;

        int * in  = (int *) safemalloc(c*sizeof(int));
        int * out = (int *) safemalloc(c*sizeof(int));

        if ( comm_rank == root )
            value = comm_size;
        else
            value = 0;

        for (int i=0 ; i<c; i++)
            in[i]  = comm_rank;

        for (int i=0 ; i<c; i++)
            out[i] = -1;

        double t0 = MPI_Wtime();
        MPI_Bcast( buffer, c, MPI_INT, root, comm );

        MPI_Reduce(void *sendbuf, void *recvbuf, int count,
                    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)

        double t1 = MPI_Wtime();
        double dt_red = t1-t0;

        int errors = 0;
        for (int i=0 ; i< c; i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            printf("MPI_Bcast had %d errors! \n", errors);
            exit(1);
        }

        if ( comm_rank == root )
            printf("MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c, dt_red, 1.0e-6*c*sizeof(int)/dt_red );

        free(out);
        free(in);
    }

    return;
}

void reducescatter_vs_reduce_scatter(MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/sizeof(int);

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

        int * buffer = (int *) safemalloc(c*sizeof(int));

        /*****************************************************/

        for (int i=0 ; i<c; i++)
            buffer[i] = value;

        t0 = MPI_Wtime();
        MPI_Bcast( buffer, c, MPI_INT, root, comm );
        t1 = MPI_Wtime();
        dt_bcast = t1-t0;

        errors = 0;
        for (int i=0 ; i<c; i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            printf("MPI_Bcast had %d errors on rank %d! \n", errors, comm_rank);
            exit(1);
        }

        if ( comm_rank == root )
            printf("MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c, t1-t0, 1.0e-6*c*sizeof(int)/(t1-t0) );

        /*****************************************************/

        if (c%comm_size==0)
        {
            int * temp = (int *) safemalloc((c/comm_size)*sizeof(int));

            for (int i=0 ; i<c; i++)
                buffer[i] = value;

            for (int i=0 ; i<(c/comm_size); i++)
                temp[i] = 0;

            t0 = MPI_Wtime();
            MPI_Scatter( buffer, c/comm_size, MPI_INT, temp, c/comm_size, MPI_INT, root, comm );
            MPI_Allgather( temp, c/comm_size, MPI_INT, buffer, c/comm_size, MPI_INT, comm );
            t1 = MPI_Wtime();
            dt_scalg = t1-t0;

            errors = 0;
            for (int i=0 ; i<c; i++)
                errors += (int) ( buffer[i] != comm_size );

            if (errors>0)
            {
                printf("MPI_Scatter+MPI_Allgather had %d errors on rank %d! \n", errors, comm_rank);
                for (int i=0 ; i<c; i++)
                    printf("rank %d: buffer[%d] = %d (correct is %d) \n", comm_rank, i, buffer[i], comm_size );
                exit(1);
            }

            if ( comm_rank == root )
                printf("MPI_Scatter+MPI_Allgather %d integers in %lf seconds (%lf MB/s) \n",
                       c, t1-t0, 1.0e-6*c*sizeof(int)/(t1-t0) );

            free(temp);
        }

        /*****************************************************/

        free(buffer);
    }

    return;
}

void reducescatter_vs_allreduce(MPI_Comm comm, int max_mem)
{
    int comm_rank = -1, comm_size = 0;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    /* testing only intergers because it really ~shouldn't~ matter */

    int max_count = max_mem/sizeof(int);

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

        int * buffer = (int *) safemalloc(c*sizeof(int));

        /*****************************************************/

        for (int i=0 ; i<c; i++)
            buffer[i] = value;

        t0 = MPI_Wtime();
        MPI_Bcast( buffer, c, MPI_INT, root, comm );
        t1 = MPI_Wtime();
        dt_bcast = t1-t0;

        errors = 0;
        for (int i=0 ; i<c; i++)
            errors += (int) ( buffer[i] != comm_size );

        if (errors>0)
        {
            printf("MPI_Bcast had %d errors on rank %d! \n", errors, comm_rank);
            exit(1);
        }

        if ( comm_rank == root )
            printf("MPI_Bcast %d integers in %lf seconds (%lf MB/s) \n",
                   c, t1-t0, 1.0e-6*c*sizeof(int)/(t1-t0) );

        /*****************************************************/

        if (c%comm_size==0)
        {
            int * temp = (int *) safemalloc((c/comm_size)*sizeof(int));

            for (int i=0 ; i<c; i++)
                buffer[i] = value;

            for (int i=0 ; i<(c/comm_size); i++)
                temp[i] = 0;

            t0 = MPI_Wtime();
            MPI_Scatter( buffer, c/comm_size, MPI_INT, temp, c/comm_size, MPI_INT, root, comm );
            MPI_Allgather( temp, c/comm_size, MPI_INT, buffer, c/comm_size, MPI_INT, comm );
            t1 = MPI_Wtime();
            dt_scalg = t1-t0;

            errors = 0;
            for (int i=0 ; i<c; i++)
                errors += (int) ( buffer[i] != comm_size );

            if (errors>0)
            {
                printf("MPI_Scatter+MPI_Allgather had %d errors on rank %d! \n", errors, comm_rank);
                for (int i=0 ; i<c; i++)
                    printf("rank %d: buffer[%d] = %d (correct is %d) \n", comm_rank, i, buffer[i], comm_size );
                exit(1);
            }

            if ( comm_rank == root )
                printf("MPI_Scatter+MPI_Allgather %d integers in %lf seconds (%lf MB/s) \n",
                       c, t1-t0, 1.0e-6*c*sizeof(int)/(t1-t0) );

            free(temp);
        }

        /*****************************************************/

        free(buffer);
    }

    return;
}
