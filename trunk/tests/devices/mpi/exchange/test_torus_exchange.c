#include <stdio.h>
#include <stdlib.h>
#include <safemalloc.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include <mpi.h>
#if defined(__bgp__) || defined(__bgq__)
#  include <mpix.h>
#else
#  warning This test should be run on a Blue Gene.
#endif

#include "safesafemalloc.h"

int posix_memalign(void **memptr, size_t alignment, size_t size);

#define ALIGNMENT 64

int main(int argc, char *argv[])
{
    int provided;
    int world_size, world_rank;

    MPI_Init_thread( &argc, &argv, MPI_THREAD_SINGLE, &provided );
    assert( provided>=MPI_THREAD_SINGLE );

    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &world_size );

    assert(world_size>1);

    int max_links = ( argc > 1 ? atoi(argv[1]) : world_size-1 );
    int max_count = ( argc > 2 ? atoi(argv[2]) : 110*1024*1024 );
    int nbrecv    = ( argc > 3 ? atoi(argv[3]) : 0 );

    int rank_ap = -1;
    int rank_am = -1;
    int rank_bp = -1;
    int rank_bm = -1;
    int rank_cp = -1;
    int rank_cm = -1;
    int rank_dp = -1; /* d is t on BGP */
    int rank_dm = -1;
    int rank_ep = -1;
    int rank_em = -1;
    int rank_fp = -1; /* f is t on BGQ */
    int rank_fm = -1;

#if defined (__bgp__)
    uint32_t xSize, ySize, zSize, tSize;
    MPIX_rank2torus( world_size-1, &xSize, &ySize, &zSize, &tSize );
    if (world_rank==0) printf("# torus size = (%d,%d,%d) \n", xSize+1, ySize+1, zSize+1 );

    uint32_t xRank, yRank, zRank, tRank;
    MPIX_rank2torus( world_rank, &xRank, &yRank, &zRank, &tRank );

    rank_ap = MPIX_torus2rank(xRank+1, yRank  , zRank  , 0);
    rank_am = MPIX_torus2rank(xRank-1, yRank  , zRank  , 0);
    rank_bp = MPIX_torus2rank(xRank  , yRank+1, zRank  , 0);
    rank_bm = MPIX_torus2rank(xRank  , yRank-1, zRank  , 0);
    rank_cp = MPIX_torus2rank(xRank  , yRank  , zRank+1, 0);
    rank_cm = MPIX_torus2rank(xRank  , yRank  , zRank-1, 0);

    printf("# from %d (%d,%d,%d) to  %d (%d,%d,%d), %d (%d,%d,%d), %d (%d,%d,%d), %d (%d,%d,%d), %d (%d,%d,%d), %d (%d,%d,%d) \n",
           world_rank, xRank  , yRank  , zRank  ,
           rank_ap   , xRank+1, yRank  , zRank  ,
           rank_am   , xRank-1, yRank  , zRank  ,
           rank_bp   , xRank  , yRank+1, zRank  ,
           rank_bm   , xRank  , yRank-1, zRank  ,
           rank_cp   , xRank  , yRank  , zRank+1,
           rank_cm   , xRank  , yRank  , zRank-1);
#elif defined(__bgq__)
    MPIX_Hardware_t hw;
    MPIX_Hardware(&hw);

    {
        int hopCoord = (hw.Coords[0]+1) % (hw.Size[0]);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hopCoord, hw.Coords[1], hw.Coords[2], hw.Coords[3], hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_ap);
    }
    {
        int hopCoord = (hw.Coords[0]==0) ? hw.Size[0] : (hw.Coords[0]-1);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hopCoord, hw.Coords[1], hw.Coords[2], hw.Coords[3], hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_am);
    }
    {
        int hopCoord = (hw.Coords[1]+1) % (hw.Size[1]);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hopCoord, hw.Coords[2], hw.Coords[3], hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_bp);
    }
    {
        int hopCoord = (hw.Coords[1]==0) ? hw.Size[1] : (hw.Coords[1]-1);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hopCoord, hw.Coords[2], hw.Coords[3], hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_bm);
    }
    {
        int hopCoord = (hw.Coords[2]+1) % (hw.Size[2]);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hw.Coords[1], hopCoord, hw.Coords[3], hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_cp);
    }
    {
        int hopCoord = (hw.Coords[2]==0) ? hw.Size[2] : (hw.Coords[2]-1);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hw.Coords[1], hopCoord, hw.Coords[3], hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_cm);
    }
    {
        int hopCoord = (hw.Coords[3]+1) % (hw.Size[3]);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hw.Coords[1], hw.Coords[2], hopCoord, hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_dp);
    }
    {
        int hopCoord = (hw.Coords[3]==0) ? hw.Size[3] : (hw.Coords[3]-1);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hw.Coords[1], hw.Coords[2], hopCoord, hw.Coords[4]};
        MPIX_Torus2rank(tempCoords, &rank_dm);
    }
    {
        int hopCoord = (hw.Coords[4]+1) % (hw.Size[4]);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hw.Coords[1], hw.Coords[2], hopCoord, hw.Coords[4]+1};
        MPIX_Torus2rank(tempCoords, &rank_ep);
    }
    {
        int hopCoord = (hw.Coords[4]==0) ? hw.Size[4] : (hw.Coords[4]-1);
        int tempCoords[MPIX_TORUS_MAX_DIMS] = { hw.Coords[0], hw.Coords[1], hw.Coords[2], hopCoord, hw.Coords[4]-1};
        MPIX_Torus2rank(tempCoords, &rank_em);
    }
#else
    rank_ap = (world_rank+1)%world_size;
    printf("#send from %d to %d \n", world_rank, rank_ap );
#endif

    int * rbuf_ap = safemalloc(max_count * sizeof(int));
    int * rbuf_am = safemalloc(max_count * sizeof(int));
    int * rbuf_bp = safemalloc(max_count * sizeof(int));
    int * rbuf_bm = safemalloc(max_count * sizeof(int));
    int * rbuf_cp = safemalloc(max_count * sizeof(int));
    int * rbuf_cm = safemalloc(max_count * sizeof(int));
    int * rbuf_dp = safemalloc(max_count * sizeof(int));
    int * rbuf_dm = safemalloc(max_count * sizeof(int));
    int * rbuf_ep = safemalloc(max_count * sizeof(int));
    int * rbuf_em = safemalloc(max_count * sizeof(int));

    int * sbuf_ap = safemalloc(max_count * sizeof(int));
    int * sbuf_am = safemalloc(max_count * sizeof(int));
    int * sbuf_bp = safemalloc(max_count * sizeof(int));
    int * sbuf_bm = safemalloc(max_count * sizeof(int));
    int * sbuf_cp = safemalloc(max_count * sizeof(int));
    int * sbuf_cm = safemalloc(max_count * sizeof(int));
    int * sbuf_dp = safemalloc(max_count * sizeof(int));
    int * sbuf_dm = safemalloc(max_count * sizeof(int));
    int * sbuf_ep = safemalloc(max_count * sizeof(int));
    int * sbuf_em = safemalloc(max_count * sizeof(int));

    if (world_rank==0) printf("#begin nonblocking send-recv 5d halo exchange test\n" );

    fflush( stdout );
    MPI_Barrier( MPI_COMM_WORLD );

    for ( int count = 1; count <= max_count ; count *= 2 )
    {
        double dt[10+1] = {1.0e9,1.0e9,1.0e9,1.0e9,1.0e9,1.0e9};

        int rc[11] = {0,0,0,0,0,0,0,0,0,0,0};
        MPI_Request req[20];

        int tag_ap = 10*count;
        int tag_am = 10*count+1;
        int tag_bp = 10*count+2;
        int tag_bm = 10*count+3;
        int tag_cp = 10*count+4;
        int tag_cm = 10*count+5;
        int tag_dp = 10*count+6;
        int tag_dm = 10*count+7;
        int tag_ep = 10*count+8;
        int tag_em = 10*count+9;

        /* links = 0 is warmup on all 10 links */
        for ( int links = 0; links <= max_links ; links++ )
        { 
            for ( int i = 0 ; i < count ; i++) rbuf_ap[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_ap[i] = tag_ap;

            for ( int i = 0 ; i < count ; i++) rbuf_am[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_am[i] = tag_am;

            for ( int i = 0 ; i < count ; i++) rbuf_bp[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_bp[i] = tag_bp;

            for ( int i = 0 ; i < count ; i++) rbuf_bm[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_bm[i] = tag_bm;

            for ( int i = 0 ; i < count ; i++) rbuf_cp[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_cp[i] = tag_cp;

            for ( int i = 0 ; i < count ; i++) rbuf_cm[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_cm[i] = tag_cm;

            for ( int i = 0 ; i < count ; i++) rbuf_dp[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_dp[i] = tag_dp;

            for ( int i = 0 ; i < count ; i++) rbuf_dm[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_dm[i] = tag_dm;

            for ( int i = 0 ; i < count ; i++) rbuf_ep[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_ep[i] = tag_ep;

            for ( int i = 0 ; i < count ; i++) rbuf_em[i] = 0;
            for ( int i = 0 ; i < count ; i++) sbuf_em[i] = tag_em;

            MPI_Barrier( MPI_COMM_WORLD );

            double t0 = MPI_Wtime();

            MPI_Irecv( rbuf_ap, count, MPI_INT, rank_ap, tag_ap, MPI_COMM_WORLD, &req[0] );
            MPI_Isend( sbuf_ap, count, MPI_INT, rank_ap, tag_ap, MPI_COMM_WORLD, &req[1] );

            MPI_Waitall( 2, req, MPI_STATUSES_IGNORE );

            double t1 = MPI_Wtime();

            MPI_Barrier( MPI_COMM_WORLD );

            /* check for correctness */
            int error = 0;

            for ( int i = 0 ; i < count ; i++)
                error += abs( tag_ap - rbuf_ap[i] );

            if (error>0) printf("%d: %d errors \n", world_rank, error );
            //assert(error==0);

            dt[links] = t1-t0;
        }
        if (world_rank==world_rank) printf("%d: send %10d bytes %12.6lf \n",
                                           world_rank, (int) sizeof(int)*count,
                                           1e-10*1*count*sizeof(int)/dt[1]);
        fflush( stdout );
    }

    free(rbuf_ap);
    free(rbuf_am);
    free(rbuf_bp);
    free(rbuf_bm);
    free(rbuf_cp);
    free(rbuf_cm);
    free(rbuf_dp);
    free(rbuf_dm);
    free(rbuf_ep);
    free(rbuf_em);

    free(sbuf_ap);
    free(sbuf_am);
    free(sbuf_bp);
    free(sbuf_bm);
    free(sbuf_cp);
    free(sbuf_cm);
    free(sbuf_dp);
    free(sbuf_dm);
    free(sbuf_ep);
    free(sbuf_em);

    MPI_Barrier( MPI_COMM_WORLD );

    if (world_rank==0) printf("#done with all tests\n");
    fflush( stdout );

    MPI_Finalize();

    return 0;
}

