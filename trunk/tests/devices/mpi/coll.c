#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

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

    int count, max_count, max_count2;

    max_count  = 16*1024*1024; /* 1B ints is 8 GB */
    max_count2 =  1*1024*1024; /* because scatter/gather will blow up mem otherwise */

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin send-recv unidirectional bandwidth test\n" );
    for ( count = 1; count < max_count2 ; count *= 2 )
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

        if ( rank == src_rank ) printf( "MPI_Send(%d->%d): %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        src_rank,dst_rank,
                                        count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin send-recv bidirectional bandwidth test\n" );
    for ( count = 1; count < max_count2 ; count *= 2 )
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

        if ( rank == src_rank ) printf( "MPI_Isend(%d->%d): %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        src_rank,dst_rank,
                                        count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        if ( rank == dst_rank ) printf( "MPI_Isend(%d->%d): %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        dst_rank,src_rank,
                                        count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buf0);
        free(buf1);
    }

    /* bcast bandwidth test */
    if ( rank == 0 ) printf( "begin bcast bandwidth test\n" );
    for ( count = 1; count < max_count ; count *= 2 )
    {
        int rc;
        
        double t0, t1;
        int * buffer;
        int root = 0;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        if ( rank == root ) 
            for ( int i = 0 ; i < count ; i++) 
                buffer[i] = i;
        else                
            for ( int i = 0 ; i < count ; i++) 
                buffer[i] = 0; 

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Bcast( buffer, count, MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( int i = 0 ; i < count ; i++) 
            assert( buffer[i] == i );

        if ( rank == root ) printf( "MPI_Bcast: %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                    count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* reduce bandwidth test */
    if ( rank == 0 ) printf( "begin reduce bandwidth test\n" );
    for ( count = 1; count < max_count ; count *= 2 )
    {
        int rc;

        double t0, t1;
        int * buffer;
        int root = 0;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        for ( int i = 0 ; i < count ; i++) 
            buffer[i] = i;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, buffer, count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( buffer      , NULL  , count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( int i = 0 ; i < count ; i++) 
                assert( buffer[i] == i*size );

        if ( rank == 0 ) printf( "MPI_Reduce(MPI_SUM): %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* allreduce bandwidth test */
    if ( rank == 0 ) printf( "begin allreduce bandwidth test\n" );
    for ( count = 1; count < max_count ; count *= 2 )
    {
        int rc;
        
        double t0, t1;
        int * buffer;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        for ( int i = 0 ; i < count ; i++) 
            buffer[i] = i;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, buffer, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( int i = 0 ; i < count ; i++) 
            assert( buffer[i] == i*size );

        if ( rank == 0 ) printf( "MPI_Allreduce(MPI_SUM): %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* gather bandwidth test */
    if ( rank == 0 ) printf( "begin gather bandwidth test\n" );
    for ( count = 1; count < max_count2 ; count *= 2 )
    {
        int rc;
        
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        posix_memalign( (void**) &snd_buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, (size_t) ALIGNMENT, (size_t) size * count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( int i = 0 ; i < count ; i++) 
            snd_buffer[i] = size;

        for ( int i = 0 ; i < ( size * count ) ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Gather( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( int i = 0 ; i < ( size * count ) ; i++ ) 
                assert( rcv_buffer[i] == size );

        if ( rank == 0 ) printf( "MPI_Gather: %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* allgather bandwidth test */
    if ( rank == 0 ) printf( "begin allgather bandwidth test\n" );
    for ( count = 1; count < max_count2 ; count *= 2 )
    {
        int rc;
        
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;

        posix_memalign( (void**) &snd_buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, (size_t) ALIGNMENT, (size_t) size * count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( int i = 0 ; i < count ; i++) 
            snd_buffer[i] = size;

        for ( int i = 0 ; i < size * count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allgather( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( int i = 0 ; i < ( size * count ) ; i++ ) 
            assert( rcv_buffer[i] == size );

        if ( rank == 0 ) printf( "MPI_Allgather: %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* scatter bandwidth test */
    if ( rank == 0 ) printf( "begin scatter bandwidth test\n" );
    for ( count = 1; count < max_count2 ; count *= 2 )
    {
        int rc;
        
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        posix_memalign( (void**) &snd_buffer, (size_t) ALIGNMENT, (size_t) size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( int i = 0 ; i < ( size * count ) ; i++) 
            snd_buffer[i] = size;

        for ( int i = 0 ; i < count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Scatter( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( int i = 0 ; i < count ; i++ ) 
                assert( rcv_buffer[i] == size );

        if ( rank == 0 ) printf( "MPI_Scatter: %lu bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    if ( rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}

