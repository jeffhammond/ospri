#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>

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

    int count = ( argc > 1 ? atoi(argv[1]) : 1 );

    MPI_Barrier( MPI_COMM_WORLD );

    /* reduce_scatter bandwidth test */
    if ( rank == 0 ) printf( "begin reduce_scatter bandwidth test\n" );
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int * counts;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
        assert( rcv_buffer != NULL );

        counts = malloc( size * sizeof(int) );
        assert( counts != NULL );

        for ( i = 0 ; i < size ; i++ ) snd_buffer[i] = 0;
        for ( i = 0 ; i < count ; i++ ) snd_buffer[ rank * count + i] = rank;
        for ( i = 0 ; i < count ; i++ ) rcv_buffer[i] = 0;
        for ( i = 0 ; i < size ; i++) counts[i] = count;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Reduce_scatter( snd_buffer, rcv_buffer, counts, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "MPI_Reduce_scatter(MPI_SUM): %u bytes transferred in %lf seconds (%lf MB/s)\n", 
                                 count * (int) sizeof(int), t1 - t0, 1e-6 * count * (int) sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
        free(counts);
    }

    if ( rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    MPI_Finalize();

    return 0;
}

