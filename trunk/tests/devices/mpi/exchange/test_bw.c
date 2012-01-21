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

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    assert(size>1);
    printf( "Hello from %d of %d processors\n", rank, size );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    int count, max_count;

    max_count = ( argc > 1 ? atoi(argv[1]) : 16*1024*1024 );

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin blocking send-recv unidirectional bandwidth test\n" );

    for ( count = 1; count < max_count ; count *= 2 )
        for ( int dst_rank = 1; dst_rank < size ; dst_rank++ )
        {
            int src_rank = 0;
            int tag = count;

            int * buffer;
            posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
            assert( buffer != NULL );

            if ( rank == dst_rank ) 
                for ( int i = 0 ; i < count ; i++) buffer[i] = 0;

            if ( rank == src_rank ) 
                for ( int i = 0 ; i < count ; i++) buffer[i] = i;

            MPI_Barrier( MPI_COMM_WORLD );

            double t0 = 0.0, t1 = 0.0;

            if ( rank == dst_rank )
            {
                 int rc = MPI_Recv( buffer, count, MPI_INT, src_rank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
                 assert ( rc == MPI_SUCCESS );
            }
            else if ( rank == src_rank ) 
            {
                 t0 = MPI_Wtime();
                 int rc = MPI_Send( buffer, count, MPI_INT, dst_rank, tag, MPI_COMM_WORLD );
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
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin nonblocking send-recv unidirectional bandwidth test\n" );

    for ( count = 1; count < max_count ; count *= 2 )
        for ( int dst_rank = 1; dst_rank < size ; dst_rank++ )
        {
            int src_rank = 0;
            int tag = count;

            MPI_Request req;

            int * buffer;
            posix_memalign( (void**) &buffer, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
            assert( buffer != NULL );

            if ( rank == dst_rank ) 
                for ( int i = 0 ; i < count ; i++) buffer[i] = 0;

            if ( rank == src_rank ) 
                for ( int i = 0 ; i < count ; i++) buffer[i] = i;

            MPI_Barrier( MPI_COMM_WORLD );

            double t0 = 0.0, t1 = 0.0;

            if ( rank == dst_rank )
            {
                 int rc1 = MPI_Irecv( buffer, count, MPI_INT, src_rank, tag, MPI_COMM_WORLD, &req );
                 int rc2 = MPI_Waitall( 1, &req, MPI_STATUSES_IGNORE ); 

                 assert ( rc1==MPI_SUCCESS && rc2==MPI_SUCCESS );
            }
            else if ( rank == src_rank ) 
            {
                 t0 = MPI_Wtime();
                 int rc1 = MPI_Isend( buffer, count, MPI_INT, dst_rank, tag, MPI_COMM_WORLD, &req );
                 int rc2 = MPI_Waitall( 1, &req, MPI_STATUSES_IGNORE ); 
                 t1 = MPI_Wtime();

                 assert ( rc1==MPI_SUCCESS && rc2==MPI_SUCCESS );
            }

            /* check for correctness */
            if ( rank == dst_rank ) 
                for ( int i = 0 ; i < count ; i++) 
                    assert( buffer[i] == i );

            /* i agree this is overkill */
            MPI_Barrier( MPI_COMM_WORLD );

            if ( rank == src_rank ) printf( "MPI_Isend(%d->%d): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                            src_rank,dst_rank,
                                            count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );

            free(buffer);
        }

    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin blocking send-recv bidirectional bandwidth test\n" );

    for ( count = 1; count < max_count ; count *= 2 )
        for ( int dst_rank = 1; dst_rank < size ; dst_rank++ )
        {
            int src_rank = 0;
 
            int tag0 = count;
            int tag1 = count+1;
 
            int * buf0;
            int * buf1;
 
            posix_memalign( (void**) &buf0, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
            assert( buf0 != NULL );
 
            posix_memalign( (void**) &buf1, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
            assert( buf1 != NULL );
 
            if ( rank == dst_rank ) 
            {
                for ( int i = 0 ; i < count ; i++) buf0[i] = 0;
                for ( int i = 0 ; i < count ; i++) buf1[i] = i;
            }
            if ( rank == src_rank ) 
            {
                for ( int i = 0 ; i < count ; i++) buf0[i] = -i;
                for ( int i = 0 ; i < count ; i++) buf1[i] = 0;
            }
 
            MPI_Barrier( MPI_COMM_WORLD );

            double t0 = 0.0, t1 = 0.0;

            if ( rank == dst_rank )
            {
                 t0 = MPI_Wtime();
                 int rc = MPI_Sendrecv( buf1, count, MPI_INT, src_rank, tag0,
                                        buf0, count, MPI_INT, src_rank, tag1,
                                        MPI_COMM_WORLD, MPI_STATUS_IGNORE );
                 t1 = MPI_Wtime();

                 assert ( rc==MPI_SUCCESS );
            }
            else if ( rank == src_rank ) 
            {
                 t0 = MPI_Wtime();
                 int rc = MPI_Sendrecv( buf0, count, MPI_INT, dst_rank, tag1,
                                        buf1, count, MPI_INT, dst_rank, tag0,
                                        MPI_COMM_WORLD, MPI_STATUS_IGNORE );
                 t1 = MPI_Wtime();

                 assert ( rc==MPI_SUCCESS );
            }
 
            /* check for correctness */
            int errors = 0;

            if ( rank == dst_rank ) 
                for ( int i = 0 ; i < count ; i++) 
                    if ( buf0[i] != -i ) errors++;

            if ( rank == src_rank ) 
                for ( int i = 0 ; i < count ; i++) 
                    if ( buf1[i] != i ) errors++;

            assert(errors==0);
 
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

    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    if ( rank == 0 ) printf( "begin nonblocking send-recv bidirectional bandwidth test\n" );

    for ( count = 1; count < max_count ; count *= 2 )
        for ( int dst_rank = 1; dst_rank < size ; dst_rank++ )
        {
            int src_rank = 0;
 
            int tag0 = count;
            int tag1 = count+1;
 
            MPI_Request req[2];
 
            int * buf0;
            int * buf1;
 
            posix_memalign( (void**) &buf0, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
            assert( buf0 != NULL );
 
            posix_memalign( (void**) &buf1, (size_t) ALIGNMENT, (size_t) count * sizeof(int) );
            assert( buf1 != NULL );
 
            if ( rank == dst_rank ) 
            {
                for ( int i = 0 ; i < count ; i++) buf0[i] = 0;
                for ( int i = 0 ; i < count ; i++) buf1[i] = i;
            }
            if ( rank == src_rank ) 
            {
                for ( int i = 0 ; i < count ; i++) buf0[i] = -i;
                for ( int i = 0 ; i < count ; i++) buf1[i] = 0;
            }
 
            MPI_Barrier( MPI_COMM_WORLD );

            double t0 = 0.0, t1 = 0.0;

            if ( rank == dst_rank )
            {
                 t0 = MPI_Wtime();
                 int rc1 = MPI_Irecv( buf0, count, MPI_INT, src_rank, tag0, MPI_COMM_WORLD, &req[0] );
                 int rc2 = MPI_Isend( buf1, count, MPI_INT, src_rank, tag1, MPI_COMM_WORLD, &req[1] );
                 int rc3 = MPI_Waitall( 2, req, MPI_STATUSES_IGNORE ); 
                 t1 = MPI_Wtime();

                 assert ( rc1==MPI_SUCCESS && rc2==MPI_SUCCESS && rc3==MPI_SUCCESS );
            }
            else if ( rank == src_rank ) 
            {
                 t0 = MPI_Wtime();
                 int rc1 = MPI_Irecv( buf1, count, MPI_INT, dst_rank, tag1, MPI_COMM_WORLD, &req[1] );
                 int rc2 = MPI_Isend( buf0, count, MPI_INT, dst_rank, tag0, MPI_COMM_WORLD, &req[0] );
                 int rc3 = MPI_Waitall( 2, req, MPI_STATUSES_IGNORE ); 
                 t1 = MPI_Wtime();

                 assert ( rc1==MPI_SUCCESS && rc2==MPI_SUCCESS && rc3==MPI_SUCCESS );
            }
 
            /* check for correctness */
            int errors = 0;

            if ( rank == dst_rank ) 
                for ( int i = 0 ; i < count ; i++) 
                    if ( buf0[i] != -i ) errors++;

            if ( rank == src_rank ) 
                for ( int i = 0 ; i < count ; i++) 
                    if ( buf1[i] != i ) errors++;

            assert(errors==0);
 
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

    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    if ( rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}

