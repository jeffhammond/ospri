#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

#define ALIGNMENT 64
#define MAX_COUNT 1024*1024

int main(int argc, char *argv[])
{
    int provided;
    int size, rank;
    int rc;

    MPI_Init_thread( &argc, &argv, MPI_THREAD_SINGLE, &provided );
    assert( provided == MPI_THREAD_SINGLE );

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    printf( "Hello from %d of %d processors\n", rank, size );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    int count;

    /* send-recv bandwidth test */
    printf( "begin send-recv bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT ; count *= 2 )
    {
        int src_rank = 0;
        int dst_rank = 1;
        int tag = 1000;

        int i;
        double t0, t1;
        int * buffer;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        MPI_Barrier( MPI_COMM_WORLD );

        if (rank==dst_rank)
        {
             for ( i = 0 ; i < count ; i++) buffer[i] = 0;

             rc = MPI_Recv( buffer, count, MPI_INT, src_rank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
             assert ( rc != MPI_SUCCESS );

             /* check for correctness */
             for ( i = 0 ; i < count ; i++) assert( buffer[i] == i );
        }
        else if (rank==src_rank) 
        {
             for ( i = 0 ; i < count ; i++) buffer[i] = i;

             t0 = MPI_Wtime();
             rc = MPI_Send( buffer, count, MPI_INT, dst_rank, tag, MPI_COMM_WORLD );
             t1 = MPI_Wtime();
             assert ( rc != MPI_SUCCESS );

             printf( "MPI_Send: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                     count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );
             fflush( stdout );
        }

        free(buffer);
    }

    /* allreduce bandwidth test */
    printf( "begin allreduce bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * buffer;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        MPI_Barrier( MPI_COMM_WORLD );

        for ( i = 0 ; i < count ; i++) buffer[i] = i;

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, buffer, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc != MPI_SUCCESS );

        printf( "MPI_Allreduce(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );
        fflush( stdout );

        for ( i = 0 ; i < count ; i++) assert( buffer[i] == i*size );

        free(buffer);
    }

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}
