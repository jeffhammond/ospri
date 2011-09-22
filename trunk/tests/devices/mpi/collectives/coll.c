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
    if ( rank == 0 ) printf( "begin send-recv bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT ; count *= 2 )
    {
        int src_rank = 0;
        int dst_rank = 1;
        int tag = 1000;

        int i;
        double t0 = 0.0, t1 = 0.0;
        int * buffer;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
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
                                        count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* bcast bandwidth test */
    if ( rank == 0 ) printf( "begin bcast bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * buffer;
        int root = 0;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        if ( rank == root ) 
            for ( i = 0 ; i < count ; i++) 
                buffer[i] = i;
        else                
            for ( i = 0 ; i < count ; i++) 
                buffer[i] = 0; 

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Bcast( buffer, count, MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++) 
            assert( buffer[i] == i );

        if ( rank == root ) printf( "MPI_Bcast: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                    count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* reduce bandwidth test */
    if ( rank == 0 ) printf( "begin reduce bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * buffer;
        int root = 0;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        for ( i = 0 ; i < count ; i++) 
            buffer[i] = i;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, buffer, count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( buffer      , NULL  , count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( i = 0 ; i < count ; i++) 
                assert( buffer[i] == i*size );

        if ( rank == 0 ) printf( "MPI_Reduce(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* allreduce bandwidth test */
    if ( rank == 0 ) printf( "begin allreduce bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * buffer;

        posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( buffer != NULL );

        for ( i = 0 ; i < count ; i++) 
            buffer[i] = i;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, buffer, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++) 
            assert( buffer[i] == i*size );

        if ( rank == 0 ) printf( "MPI_Allreduce(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* gather bandwidth test */
    if ( rank == 0 ) printf( "begin gather bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT/size ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        posix_memalign( (void**) &snd_buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, (size_t) ALIGNMENT, (size_t) size * count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < count ; i++) 
            snd_buffer[i] = size;

        for ( i = 0 ; i < ( size * count ) ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Gather( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( i = 0 ; i < ( size * count ) ; i++ ) 
                assert( rcv_buffer[i] == size );

        if ( rank == 0 ) printf( "MPI_Gather: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* allgather bandwidth test */
    if ( rank == 0 ) printf( "begin allgather bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT/size ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;

        posix_memalign( (void**) &snd_buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, (size_t) ALIGNMENT, (size_t) size * count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < count ; i++) 
            snd_buffer[i] = size;

        for ( i = 0 ; i < size * count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allgather( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < ( size * count ) ; i++ ) 
            assert( rcv_buffer[i] == size );

        if ( rank == 0 ) printf( "MPI_Allgather: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * sizeof(int), t1 - t0, 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* scatter bandwidth test */
    if ( rank == 0 ) printf( "begin scatter bandwidth test\n" );
    for ( count = 1; count < MAX_COUNT/size ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        posix_memalign( (void**) &snd_buffer, (size_t) ALIGNMENT, (size_t) size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        posix_memalign( (void**) &rcv_buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++) 
            snd_buffer[i] = size;

        for ( i = 0 ; i < count ; i++) 
            rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Scatter( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) 
            assert( rcv_buffer[i] == size );

        if ( rank == 0 ) printf( "MPI_Scatter: %u bytes transferred in %lf seconds (%lf MB/s)\n", 
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

