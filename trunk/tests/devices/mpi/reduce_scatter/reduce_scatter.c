#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>

#if MPI_VERSION == 2
#  if MPI_SUBVERSION == 0
#     error "I don't know when MPI_Reduce_scatter was added to the MPI standard :-("
#  elif MPI_SUBVERSION == 1
#     define MPI21
#  endif
#else
#  error "You need MPI-2!"
#endif

#define ALIGNMENT 64
#define MIN_COUNT 1
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

    //printf( "Hello from %d of %d processors\n", rank, size );
    //fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    int count;

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin send-recv bandwidth test\n" );
    if ( size > 1 )
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int src_rank = 0;
        int dst_rank = 1;
        int tag = 1000;

        int i;
        double t0 = 0.0, t1 = 0.0;
        int * buffer;

        posix_memalign( (void**) &buffer, ALIGNMENT, count * (int) sizeof(int) );
        assert( buffer != NULL );

        if ( rank == dst_rank ) 
            for ( i = 0 ; i < count ; i++) 
                buffer[i] = 0;
        if ( rank == src_rank ) 
            for ( i = 0 ; i < count ; i++) 
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
            for ( i = 0 ; i < count ; i++) 
                assert( buffer[i] == i );

        /* i agree this is overkill */
        MPI_Barrier( MPI_COMM_WORLD );

        if ( rank == src_rank ) printf( "MPI_Send: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                        count * (int) sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* reduce bandwidth test */
    if ( rank == 0 ) printf( "begin reduce bandwidth test\n" );
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * buffer;
        int root = 0;

        posix_memalign( (void**) &buffer, ALIGNMENT, size * count * (int) sizeof(int) );
        assert( buffer != NULL );
        
        for ( i = 0 ; i < ( size * count ) ; i++ ) 
            buffer[i] = 0;

        for ( i = 0 ; i < count ; i++ ) 
            buffer[ rank * count + i ] = rank;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, buffer, size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( buffer      , NULL  , size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( r = 0 ; r < size ; r++ ) 
                for ( i = 0 ; i < count ; i++ )
                    //printf( "reduced: r=%d i=%d r*c+i=%d buffer[.]=%d\n", r, i, r * count + i , buffer[ r * count + i ] );
                    assert( buffer[ r * count + i ] == r );

        if ( rank == 0 ) printf( "MPI_Reduce(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 size * count * (int) sizeof(int), t1 - t0, 1e-6 * size * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* allreduce bandwidth test */
    if ( rank == 0 ) printf( "begin allreduce bandwidth test\n" );
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * buffer;

        posix_memalign( (void**) &buffer, ALIGNMENT, size * count * (int) sizeof(int) );
        assert( buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) 
            buffer[i] = 0;

        for ( i = 0 ; i < count ; i++ ) 
            buffer[ rank * count + i ] = rank;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, buffer, size * count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( r = 0 ; r < size ; r++ ) 
            for ( i = 0 ; i < count ; i++ )
                assert( buffer[ r * count + i ] == r );

        if ( rank == 0 ) printf( "MPI_Allreduce(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 size * count * (int) sizeof(int), t1 - t0, 1e-6 * size * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* scatter bandwidth test */
    if ( rank == 0 ) printf( "begin scatter bandwidth test\n" );
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        posix_memalign( (void**) &snd_buffer, ALIGNMENT, size * count * (int) sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, ALIGNMENT, count * (int) sizeof(int) );
        assert( rcv_buffer != NULL );

        if ( rank == root )
        {
            for ( r = 0 ; r < size ; r++ ) 
                for ( i = 0 ; i < count ; i++ )
                    snd_buffer[ r * count + i ] = r;
        }
        else
        {
            for ( r = 0 ; r < size ; r++ ) 
                for ( i = 0 ; i < count ; i++ )
                    snd_buffer[ r * count + i ] = -1;
        }

        for ( i = 0 ; i < count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Scatter( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) 
            assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "MPI_Scatter: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * (int) sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* reduce+scatter bandwidth test */
    if ( rank == 0 ) printf( "begin reduce+scatter bandwidth test\n" );
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        posix_memalign( (void**) &snd_buffer, ALIGNMENT, size * count * (int) sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, ALIGNMENT, count * (int) sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) 
            snd_buffer[i] = 0;

        for ( i = 0 ; i < count ; i++ ) 
            snd_buffer[ rank * count + i ] = rank;

        for ( i = 0 ; i < count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, snd_buffer, size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( snd_buffer      , NULL  , size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        if ( rc == MPI_SUCCESS )
                            rc = MPI_Scatter( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) 
            assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "MPI_Reduce(MPI_SUM)+Scatter: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * (int) sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* reduce_scatter bandwidth test */
    if ( rank == 0 ) printf( "begin reduce_scatter bandwidth test\n" );
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int * counts;

        posix_memalign( (void**) &snd_buffer, ALIGNMENT, size * count * (int) sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, ALIGNMENT, count * (int) sizeof(int) );
        assert( rcv_buffer != NULL );

        posix_memalign( (void**) &counts, ALIGNMENT, count * (int) sizeof(int) );
        assert( counts != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) 
            snd_buffer[i] = 0;

        for ( i = 0 ; i < count ; i++ ) 
            snd_buffer[ rank * count + i ] = rank;

        for ( i = 0 ; i < count ; i++) 
            rcv_buffer[i] = 0;

        for ( i = 0 ; i < count ; i++) 
            counts[i] = count;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
#ifdef MPI21
        rc = MPI_Reduce_scatter( snd_buffer, rcv_buffer, counts, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
#else
        rc = MPI_Reduce_scatter_block( snd_buffer, rcv_buffer, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
#endif
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) 
            assert( rcv_buffer[i] == rank );

#ifdef MPI21
        if ( rank == 0 ) printf( "MPI_Reduce_scatter(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * (int) sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );
#else
        if ( rank == 0 ) printf( "MPI_Reduce_scatter_block(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * (int) sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );
#endif

        free(snd_buffer);
        free(rcv_buffer);
        free(counts);
    }

    /* allreduce bandwidth test */
    if ( rank == 0 ) printf( "begin reduce_scatter via allreduce+memcpy bandwidth test\n" );
    for ( count = MIN_COUNT; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        int error = 0;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;

        posix_memalign( (void**) &snd_buffer, ALIGNMENT, size * count * (int) sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, ALIGNMENT, count * (int) sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) 
            snd_buffer[i] = 0;

        for ( i = 0 ; i < count ; i++ ) 
            snd_buffer[ rank * count + i ] = rank;

        for ( i = 0 ; i < count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, snd_buffer, size * count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = snd_buffer[ rank * count + i ];
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) 
            assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "MPI_Allreduce(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 size * count * (int) sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    if ( rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    MPI_Finalize();

    return 0;
}

