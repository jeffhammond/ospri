#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <mpi.h>

int posix_memalign(void **memptr, size_t alignment, size_t size);

#define ALIGNMENT 64

#ifdef __bgp__
#  include <mpix.h>
#else
#  warning This test should be run on BGP.
#endif

int main(int argc, char *argv[])
{
    int provided;
    int world_size, world_rank;

    MPI_Init_thread( &argc, &argv, MPI_THREAD_SINGLE, &provided );
    assert( provided == MPI_THREAD_SINGLE );

    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &world_size );
#ifdef __bgp__
    assert(world_size>=8);
#else
    assert(world_size>1);
#endif

    int max_links;
    max_links = ( argc > 1 ? atoi(argv[1]) : 6 );

    int max_count;
    max_count = ( argc > 2 ? atoi(argv[2]) : 16*1024*1024 );

#ifdef __bgp__
    uint32_t xSize, ySize, zSize, tSize;
    MPIX_rank2torus( world_size-1, &xSize, &ySize, &zSize, &tSize );
    if (world_rank==0) printf("torus size = (%d,%d,%d) \n", xSize+1, ySize+1, zSize+1 );

    uint32_t xRank, yRank, zRank, tRank;
    MPIX_rank2torus( world_rank, &xRank, &yRank, &zRank, &tRank );
    //printf("I am %d (%d,%d,%d). \n", world_rank, xRank, yRank, zRank );

    uint32_t xRank_xp = ( xRank==xSize ? 0 : xRank+1 );
    uint32_t yRank_yp = ( yRank==ySize ? 0 : yRank+1 );
    uint32_t zRank_zp = ( zRank==zSize ? 0 : zRank+1 );

    uint32_t xRank_xm = ( xRank==0 ? xSize : xRank-1 );
    uint32_t yRank_ym = ( yRank==0 ? ySize : yRank-1 );
    uint32_t zRank_zm = ( zRank==0 ? zSize : zRank-1 );

    printf("I am %d (%d,%d,%d). (X+,Y+,Z+) = (%d,%d,%d) \n", world_rank, xRank, yRank, zRank, xRank_xp, yRank_yp, zRank_zp );
    printf("I am %d (%d,%d,%d). (X-,Y-,Z-) = (%d,%d,%d) \n", world_rank, xRank, yRank, zRank, xRank_xm, yRank_ym, zRank_zm );

    int rank_xp = MPIX_torus2rank( xRank_xp, yRank,    zRank,    tRank );
    int rank_xm = MPIX_torus2rank( xRank_xm, yRank,    zRank,    tRank );
    int rank_yp = MPIX_torus2rank( xRank,    yRank_yp, zRank,    tRank );
    int rank_ym = MPIX_torus2rank( xRank,    yRank_ym, zRank,    tRank );
    int rank_zp = MPIX_torus2rank( xRank,    yRank,    zRank_zp, tRank );
    int rank_zm = MPIX_torus2rank( xRank,    yRank,    zRank_zm, tRank );
#else
    int rank_xp = (world_rank+1)%world_size;
    int rank_xm = (world_rank+1)%world_size;
    int rank_yp = (world_rank+1)%world_size;
    int rank_ym = (world_rank+1)%world_size;
    int rank_zp = (world_rank+1)%world_size;
    int rank_zm = (world_rank+1)%world_size;
#endif

    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    if ( world_rank == 0 ) printf( "begin nonblocking send-recv 3d halo exchange test\n" );

    for ( int count = 1; count <= max_count ; count *= 2 )
    for ( int links = 1; links <= max_links ; links++ )
    { 
        int rc[13];

        int tag_xp = 6*count;
        int tag_xm = 6*count+1;
        int tag_yp = 6*count+2;
        int tag_ym = 6*count+3;
        int tag_zp = 6*count+4;
        int tag_zm = 6*count+5;
 
        MPI_Request req[12];
 
        int * rbuf_xp = malloc((size_t) count * sizeof(int));
        int * rbuf_xm = malloc((size_t) count * sizeof(int));
        int * rbuf_yp = malloc((size_t) count * sizeof(int));
        int * rbuf_ym = malloc((size_t) count * sizeof(int));
        int * rbuf_zp = malloc((size_t) count * sizeof(int));
        int * rbuf_zm = malloc((size_t) count * sizeof(int));
 
        assert( rbuf_xp != NULL && rbuf_xm != NULL && rbuf_yp != NULL && rbuf_ym != NULL && rbuf_zp != NULL && rbuf_zm != NULL);
 
        int * sbuf_xp = malloc((size_t) count * sizeof(int));
        int * sbuf_xm = malloc((size_t) count * sizeof(int));
        int * sbuf_yp = malloc((size_t) count * sizeof(int));
        int * sbuf_ym = malloc((size_t) count * sizeof(int));
        int * sbuf_zp = malloc((size_t) count * sizeof(int));
        int * sbuf_zm = malloc((size_t) count * sizeof(int));
 
        assert( sbuf_xp != NULL && sbuf_xm != NULL && sbuf_yp != NULL && sbuf_ym != NULL && sbuf_zp != NULL && sbuf_zm != NULL);

        for ( int i = 0 ; i < count ; i++) rbuf_xp[i] = 0;
        for ( int i = 0 ; i < count ; i++) rbuf_xm[i] = 0;
        for ( int i = 0 ; i < count ; i++) rbuf_yp[i] = 0;
        for ( int i = 0 ; i < count ; i++) rbuf_ym[i] = 0;
        for ( int i = 0 ; i < count ; i++) rbuf_zp[i] = 0;
        for ( int i = 0 ; i < count ; i++) rbuf_zm[i] = 0;
 
        for ( int i = 0 ; i < count ; i++) sbuf_xp[i] = 6*i;
        for ( int i = 0 ; i < count ; i++) sbuf_xm[i] = 6*i+1;
        for ( int i = 0 ; i < count ; i++) sbuf_yp[i] = 6*i+2;
        for ( int i = 0 ; i < count ; i++) sbuf_ym[i] = 6*i+3;
        for ( int i = 0 ; i < count ; i++) sbuf_zp[i] = 6*i+4;
        for ( int i = 0 ; i < count ; i++) sbuf_zm[i] = 6*i+5;
 
        MPI_Barrier( MPI_COMM_WORLD );

        double t0 = MPI_Wtime();

        if (links>0)
        {
            printf("rank %d recv from rank %d \n", world_rank, rank_xp);
            printf("rank %d send  to  rank %d \n", world_rank, rank_xm);
            rc[0]  = MPI_Irecv( rbuf_xp, count, MPI_INT, rank_xp, tag_xp, MPI_COMM_WORLD, &req[0] );
            rc[1]  = MPI_Isend( sbuf_xm, count, MPI_INT, rank_xm, tag_xp, MPI_COMM_WORLD, &req[1] );
        }
        if (links>1)
        {
            printf("rank %d recv from rank %d \n", world_rank, rank_xm);
            printf("rank %d send  to  rank %d \n", world_rank, rank_xp);
            rc[2]  = MPI_Irecv( rbuf_xm, count, MPI_INT, rank_xm, tag_xm, MPI_COMM_WORLD, &req[2] );
            rc[3]  = MPI_Isend( sbuf_xp, count, MPI_INT, rank_xp, tag_xm, MPI_COMM_WORLD, &req[3] );
        }
        if (links>2)
        {
            printf("rank %d recv from rank %d \n", world_rank, rank_yp);
            printf("rank %d send  to  rank %d \n", world_rank, rank_ym);
            rc[4]  = MPI_Irecv( rbuf_yp, count, MPI_INT, rank_yp, tag_yp, MPI_COMM_WORLD, &req[4] );
            rc[5]  = MPI_Isend( sbuf_ym, count, MPI_INT, rank_ym, tag_yp, MPI_COMM_WORLD, &req[5] );
        }
        if (links>3)
        {
            printf("rank %d recv from rank %d \n", world_rank, rank_ym);
            printf("rank %d send  to  rank %d \n", world_rank, rank_yp);
            rc[6]  = MPI_Irecv( rbuf_ym, count, MPI_INT, rank_ym, tag_ym, MPI_COMM_WORLD, &req[6] );
            rc[7]  = MPI_Isend( sbuf_yp, count, MPI_INT, rank_yp, tag_ym, MPI_COMM_WORLD, &req[7] );
        }
        if (links>4)
        {
            printf("rank %d recv from rank %d \n", world_rank, rank_zp);
            printf("rank %d send  to  rank %d \n", world_rank, rank_zm);
            rc[8]  = MPI_Irecv( rbuf_zp, count, MPI_INT, rank_zp, tag_zp, MPI_COMM_WORLD, &req[8] );
            rc[9]  = MPI_Isend( sbuf_zm, count, MPI_INT, rank_zm, tag_zp, MPI_COMM_WORLD, &req[9] );
        }
        if (links>5)
        {
            printf("rank %d recv from rank %d \n", world_rank, rank_zm);
            printf("rank %d send  to  rank %d \n", world_rank, rank_zp);
            rc[10] = MPI_Irecv( rbuf_zm, count, MPI_INT, rank_zm, tag_zm, MPI_COMM_WORLD, &req[10] );
            rc[11] = MPI_Isend( sbuf_zp, count, MPI_INT, rank_zp, tag_zm, MPI_COMM_WORLD, &req[11] );
        }

        printf("%d: waitall on %d requests \n", world_rank, 2*links );
        rc[12] = MPI_Waitall( 2*links, req, MPI_STATUSES_IGNORE ); 

        double t1 = MPI_Wtime();

        MPI_Barrier( MPI_COMM_WORLD );

        for ( int i = 0 ; i < (2*links) ; i++ ) assert( rc[i]==MPI_SUCCESS );
        assert( rc[12]==MPI_SUCCESS );
 
#if 0
        for ( int i = 0 ; i < count ; i++) 
            printf("rank %d: i=%d rbuf_xm = %d rbuf_xp = %d rbuf_ym = %d rbuf_yp = %d rbuf_zm = %d rbuf_zp = %d \n",
                    world_rank, i, rbuf_xm[i], rbuf_xp[i], rbuf_ym[i], rbuf_yp[i], rbuf_zm[i], rbuf_zp[i] );
#endif

        /* check for correctness */
        int error = 0;

        for ( int i = 0 ; i < count ; i++) 
            error += abs( 6*i   - rbuf_xm[i] );

        for ( int i = 0 ; i < count ; i++) 
            error += abs( 6*i+1 - rbuf_xp[i] );

        for ( int i = 0 ; i < count ; i++) 
            error += abs( 6*i+2 - rbuf_ym[i] );

        for ( int i = 0 ; i < count ; i++) 
            error += abs( 6*i+3 - rbuf_yp[i] );

        for ( int i = 0 ; i < count ; i++) 
            error += abs( 6*i+4 - rbuf_zm[i] );

        for ( int i = 0 ; i < count ; i++) 
            error += abs( 6*i+5 - rbuf_zp[i] );

        assert(error==0);
 
        /* i agree this is overkill */
        MPI_Barrier( MPI_COMM_WORLD );

        free(rbuf_xp);
        free(rbuf_xm);
        free(rbuf_yp);
        free(rbuf_ym);
        free(rbuf_zp);
        free(rbuf_zm);
               
        free(sbuf_xp);
        free(sbuf_xm);
        free(sbuf_yp);
        free(sbuf_ym);
        free(sbuf_zp);
        free(sbuf_zm);

        printf("%d: send %d bytes on 6 links, BW = %lf MB/s \n", world_rank, count, 1e-6*6*count/(t1-t0) );
        fflush( stdout );
        MPI_Barrier( MPI_COMM_WORLD );
    }

    if ( world_rank == 0 ) printf( "done with all tests\n" );
    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;
}

