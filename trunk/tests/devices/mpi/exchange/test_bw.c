#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <mpi.h>

int posix_memalign(void **memptr, size_t alignment, size_t size);

#define ALIGNMENT 64

int main(int argc, char *argv[])
{
    int provided;
    int size, rank;

    MPI_Init_thread( &argc, &argv, MPI_THREAD_SINGLE, &provided );
    assert( provided == MPI_THREAD_SINGLE );

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    printf( "Hello from %d of %d processors\n", rank, size );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    int count, max_count;

    max_count = 16*1024*1024;

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin send-recv unidirectional bandwidth test\n" );
    for ( count = 1; count < max_count ; count *= 2 )
    for ( int dst_rank = 1; dst_rank < size ; dst_rank++ )
    {
        int rc;

        int src_rank = 0;
        int tag = 1000;

        double t0 = 0.0, t1 = 0.0;
        int * buffer;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        if ( rank == dst_rank ) 
            for ( int i = 0 ; i < count ; i++) 
                buffer[i] = 0;
        if ( rank == src_rank ) 
            for ( int i = 0 ; i < count ; i++) 
                buffer[i] = i;

        MPI_Barrier( MPI_COMM_WORLD );

        if ( rank == dst_rank )
        {
             rc = MPI_Recv( buffer, count, MPI_INT, src_rank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
             assert ( rc == MPI_SUCCESS );
        }
        else if ( rank == src_rank ) 
        {
             t0 = MPI_Wtime();
             rc = MPI_Send( buffer, count, MPI_INT, dst_rank, tag, MPI_COMM_WORLD );
             t1 = MPI_Wtime();
             assert ( rc == MPI_SUCCESS );
        }

        /* check for correctness */
        if ( rank == dst_rank ) 
            for ( int i = 0 ; i < count ; i++) 
                assert( buffer[i] == i );

        /* i agree this is overkill */
        MPI_Barrier( MPI_COMM_WORLD );

        if ( rank == src_rank ) printf( "MPI_Send(%d->%d): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        src_rank,dst_rank,
                                        count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin send-recv bidirectional bandwidth test\n" );
    for ( count = 1; count < max_count ; count *= 2 )
    for ( int dst_rank = 1; dst_rank < size ; dst_rank++ )
    {
        int src_rank = 0;

        int rc;

        int tag0 = 0;
        int tag1 = 1;

        double t0 = 0.0, t1 = 0.0;

        int * buf0;
        int * buf1;

        MPI_Request req[2];

        posix_memalign( (void**) &buf0, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buf0 != NULL );

        posix_memalign( (void**) &buf1, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buf1 != NULL );

        if ( rank == dst_rank ) 
        {
            for ( int i = 0 ; i < count ; i++) 
                buf0[i] = 0;
            for ( int i = 0 ; i < count ; i++) 
                buf1[i] = 0;
        }
        if ( rank == src_rank ) 
        {
            for ( int i = 0 ; i < count ; i++) 
                buf0[i] = 0;
            for ( int i = 0 ; i < count ; i++) 
                buf1[i] = 0;
        }

        MPI_Barrier( MPI_COMM_WORLD );

        if ( rank == dst_rank )
        {
             t0 = MPI_Wtime();

             rc = MPI_Irecv( buf0, count, MPI_INT, src_rank, tag0, MPI_COMM_WORLD, &req[0] );
             assert ( rc == MPI_SUCCESS );

             rc = MPI_Isend( buf1, count, MPI_INT, src_rank, tag1, MPI_COMM_WORLD, &req[1] );
             assert ( rc == MPI_SUCCESS );

             rc = MPI_Waitall(2,req,MPI_STATUSES_IGNORE); 
             assert ( rc == MPI_SUCCESS );

             t1 = MPI_Wtime();
        }
        else if ( rank == src_rank ) 
        {
             t0 = MPI_Wtime();

             rc = MPI_Irecv( buf1, count, MPI_INT, dst_rank, tag1, MPI_COMM_WORLD, &req[1] );
             assert ( rc == MPI_SUCCESS );

             rc = MPI_Isend( buf0, count, MPI_INT, dst_rank, tag0, MPI_COMM_WORLD, &req[0] );
             assert ( rc == MPI_SUCCESS );

             rc = MPI_Waitall(2,req,MPI_STATUSES_IGNORE); 
             assert ( rc == MPI_SUCCESS );

             t1 = MPI_Wtime();
        }

        /* check for correctness */
        if ( rank == dst_rank ) 
            for ( int i = 0 ; i < count ; i++) 
                assert( buf0[i] == 0 );
        if ( rank == src_rank ) 
            for ( int i = 0 ; i < count ; i++) 
                assert( buf1[i] == 0 );

        /* i agree this is overkill */
        MPI_Barrier( MPI_COMM_WORLD );

        if ( rank == src_rank ) printf( "MPI_Isend(%d->%d): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        src_rank,dst_rank,
                                        count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );

        if ( rank == dst_rank ) printf( "MPI_Isend(%d->%d): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        dst_rank,src_rank,
                                        count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );

        free(buf0);
        free(buf1);
    }

    if ( rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}

