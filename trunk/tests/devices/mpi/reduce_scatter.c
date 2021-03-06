#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>

#ifdef __bgp__
#  include <dcmf.h>
#  include <spi/kernel_interface.h>
#  include <common/bgp_personality.h>
#  include <common/bgp_personality_inlines.h>
#endif

int main(int argc, char *argv[])
{
    int provided;
    int size, rank;
    int rc;

    MPI_Init_thread( &argc, &argv, MPI_THREAD_SINGLE, &provided );
    //assert( provided == MPI_THREAD_SINGLE );

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    //printf( "Hello from %d of %d processors\n", rank, size );
    //fflush( stdout );

    int count;
    int min_count = (argc > 1 ? atoi(argv[1]) : 1);
    int max_count = (argc > 2 ? atoi(argv[2]) : 1024);

    if ( rank == 0 ) printf( "world size=%d min_count=%d max_count=%d\n", size , min_count , max_count );
#ifdef __bgp__
    {
        _BGP_Personality_t pers;
        char* mode = "WTF ";
        char* torusX = "?";
        char* torusY = "?";
        char* torusZ = "?";
        int sizeX = -1;
        int sizeY = -1;
        int sizeZ = -1;

        if (rank == 0)
        {
             Kernel_GetPersonality(&pers, sizeof(pers));
          
                  if ( BGP_Personality_processConfig(&pers) == _BGP_PERS_PROCESSCONFIG_SMP ) mode = "SMP ";
             else if ( BGP_Personality_processConfig(&pers) == _BGP_PERS_PROCESSCONFIG_VNM ) mode = "VNM ";
             else if ( BGP_Personality_processConfig(&pers) == _BGP_PERS_PROCESSCONFIG_2x2 ) mode = "DUAL";
          
             torusX = BGP_Personality_isTorusX(&pers) ? "T" : "F";
             torusY = BGP_Personality_isTorusY(&pers) ? "T" : "F";
             torusZ = BGP_Personality_isTorusZ(&pers) ? "T" : "F";
          
             sizeX = BGP_Personality_xSize(&pers);
             sizeY = BGP_Personality_ySize(&pers);
             sizeZ = BGP_Personality_zSize(&pers);
          
             printf("mode %s topology (%2d,%2d,%2d) torus? (%1s,%1s,%1s)\n",
                     mode, sizeX, sizeY, sizeZ, torusX, torusY, torusZ );
        }
    }
#endif
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    /* send-recv bandwidth test */
    if ( rank == 0 ) printf( "begin send-recv bandwidth test\n" );
    if ( size > 1 )
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int src_rank = 0;
        int dst_rank = 1;
        int tag = 1000;

        int i;
        double t0 = 0.0, t1 = 0.0;
        int * buffer;

        buffer = malloc( count * sizeof(int) );
        assert( buffer != NULL );

        if ( rank == dst_rank ) 
            for ( i = 0 ; i < count ; i++) buffer[i] = 0;
        if ( rank == src_rank ) 
            for ( i = 0 ; i < count ; i++) buffer[i] = i;

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
            for ( i = 0 ; i < count ; i++) assert( buffer[i] == i );

        /* i agree this is overkill */
        MPI_Barrier( MPI_COMM_WORLD );

        if ( rank == src_rank ) printf( "%35s: %17u bytes transferred in %6.2e seconds (%6.2e MB/s)\n",
                                        "MPI_Send",
                                        count * (int) sizeof(int),
                                        t1 - t0, 
                                        1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* reduce bandwidth test */
    if ( rank == 0 ) printf( "begin reduce bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * buffer;
        int root = 0;

        buffer = malloc( size * count * sizeof(int) );
        assert( buffer != NULL );
        
        for ( i = 0 ; i < ( size * count ) ; i++ ) buffer[i] = 0; 
        for ( i = 0 ; i < count ; i++ ) buffer[ rank * count + i ] = rank;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, buffer, size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( buffer      , NULL  , size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        if ( rank == root ) 
            for ( r = 0 ; r < size ; r++ ) 
                for ( i = 0 ; i < count ; i++ )
                    assert( buffer[ r * count + i ] == r );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Reduce(MPI_SUM)",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* allreduce bandwidth test */
    if ( rank == 0 ) printf( "begin allreduce bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * buffer;

        buffer = malloc( size * count * sizeof(int) );
        assert( buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) buffer[i] = 0; 
        for ( i = 0 ; i < count ; i++ ) buffer[ rank * count + i ] = rank;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, buffer, size * count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( r = 0 ; r < size ; r++ ) 
            for ( i = 0 ; i < count ; i++ )
                assert( buffer[ r * count + i ] == r );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Allreduce(MPI_SUM)",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(buffer);
    }

    /* scatter bandwidth test */
    if ( rank == 0 ) printf( "begin scatter bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
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

        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Scatter( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Scatter",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* scatterv bandwidth test */
    if ( rank == 0 ) printf( "begin scatterv bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i, r;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int * counts;
        int * displs;
        int root = 0;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
        assert( rcv_buffer != NULL );

        counts = malloc( size * sizeof(int) );
        assert( counts != NULL );

        displs = malloc( size * sizeof(int) );
        assert( displs != NULL );

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
        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = 0;
        for ( i = 0 ; i < size ; i++) counts[i] = count;
        for ( i = 0 ; i < size ; i++) displs[i] = i*count;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Scatterv( snd_buffer, counts, displs, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Scatterv",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
        free(displs);
        free(counts);
    }

    /* reduce+scatter bandwidth test */
    if ( rank == 0 ) printf( "begin reduce+scatter bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int root = 0;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) snd_buffer[i] = 0; 
        for ( i = 0 ; i < count ; i++ ) snd_buffer[ rank * count + i ] = rank; 
        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, snd_buffer, size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( snd_buffer      , NULL  , size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        if ( rc == MPI_SUCCESS )
                            rc = MPI_Scatter( snd_buffer, count, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Reduce(MPI_SUM)+Scatter",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    /* reduce+scatterv bandwidth test */
    if ( rank == 0 ) printf( "begin reduce+scatterv bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;
        int * counts;
        int * displs;
        int root = 0;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
        assert( rcv_buffer != NULL );

        counts = malloc( size * sizeof(int) );
        assert( counts != NULL );

        displs = malloc( size * sizeof(int) );
        assert( displs != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) snd_buffer[i] = 0; 
        for ( i = 0 ; i < count ; i++ ) snd_buffer[ rank * count + i ] = rank; 
        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = 0;
        for ( i = 0 ; i < size ; i++) counts[i] = count;
        for ( i = 0 ; i < size ; i++) displs[i] = i*count;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        if ( rank == root ) rc = MPI_Reduce( MPI_IN_PLACE, snd_buffer, size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        else                rc = MPI_Reduce( snd_buffer      , NULL  , size * count, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
        if ( rc == MPI_SUCCESS )
                            rc = MPI_Scatterv( snd_buffer, counts, displs, MPI_INT, rcv_buffer, count , MPI_INT, root, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Reduce(MPI_SUM)+Scatterv",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
        free(counts);
        free(displs);
    }

    /* reduce_scatter bandwidth test */
    if ( rank == 0 ) printf( "begin reduce_scatter bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
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

        for ( i = 0 ; i < ( size * count ) ; i++ ) snd_buffer[i] = 0;
        for ( i = 0 ; i < count ; i++ ) snd_buffer[ rank * count + i] = rank;
        for ( i = 0 ; i < count ; i++ ) rcv_buffer[i] = 0;
        for ( i = 0 ; i < size ; i++) counts[i] = count;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Reduce_scatter( snd_buffer, rcv_buffer, counts, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Reduce_scatter(MPI_SUM)",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
        free(counts);
    }

#if ( MPI_VERSION == 2 && MPI_SUBVERSION == 2 ) || MPI_VERSION >= 3
    /* reduce_scatter_block bandwidth test */
    if ( rank == 0 ) printf( "begin reduce_scatter_block bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) snd_buffer[i] = 0;
        for ( i = 0 ; i < count ; i++ ) snd_buffer[ rank * count + i ] = rank;
        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Reduce_scatter_block( snd_buffer, rcv_buffer, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Reduce_scatter_block(MPI_SUM)",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }
#else
#    warning MPI_Reduce_scatter_block requires MPI-2.2
#endif

    /* allreduce bandwidth test */
    if ( rank == 0 ) printf( "begin reduce_scatter via allreduce+memcpy bandwidth test\n" );
    for ( count = min_count; count < max_count ; count *= 2 )
    {
        int i;
        double t0, t1;
        int * snd_buffer;
        int * rcv_buffer;

        snd_buffer = malloc( size * count * sizeof(int) );
        assert( snd_buffer != NULL );

        rcv_buffer = malloc( count * sizeof(int) );
        assert( rcv_buffer != NULL );

        for ( i = 0 ; i < ( size * count ) ; i++ ) snd_buffer[i] = 0; 
        for ( i = 0 ; i < count ; i++ ) snd_buffer[ rank * count + i ] = rank; 
        for ( i = 0 ; i < count ; i++) rcv_buffer[i] = 0;

        MPI_Barrier( MPI_COMM_WORLD );

        t0 = MPI_Wtime();
        rc = MPI_Allreduce( MPI_IN_PLACE, snd_buffer, size * count, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        //for ( i = 0 ; i < count ; i++) rcv_buffer[i] = snd_buffer[ rank * count + i ];
        memcpy(&rcv_buffer[0],&snd_buffer[rank*count],count*sizeof(int));
        t1 = MPI_Wtime();
        assert ( rc == MPI_SUCCESS );

        for ( i = 0 ; i < count ; i++ ) assert( rcv_buffer[i] == rank );

        if ( rank == 0 ) printf( "%35s: %10u bytes (%7u bytes per rank) transferred in %6.2e seconds (%6.2e MB/s or %6.2e MB/s bytes per rank)\n",
                                 "MPI_Allreduce(MPI_SUM)+memcpy",
                                 size * count * (int) sizeof(int), count * (int) sizeof(int), 
                                 t1 - t0, 
                                 1e-6 * size * count * sizeof(int) / (t1-t0),
                                 1e-6 * count * sizeof(int) / (t1-t0) );

        free(snd_buffer);
        free(rcv_buffer);
    }

    if ( rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    MPI_Finalize();

    return 0;
}

