/***************************************************************************

                  COPYRIGHT

The following is a notice of limited availability of the code, and disclaimer
which must be included in the prologue of the code and in all source listings
of the code.

Copyright Notice
 + 2009 University of Chicago

Permission is hereby granted to use, reproduce, prepare derivative works, and
to redistribute to others.  This software was authored by:

Jeff R. Hammond
Leadership Computing Facility
Argonne National Laboratory
Argonne IL 60439 USA
phone: (630) 252-5381
e-mail: jhammond@mcs.anl.gov

                  GOVERNMENT LICENSE

Portions of this material resulted from work developed under a U.S.
Government Contract and are subject to the following license: the Government
is granted for itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable worldwide license in this computer software to reproduce, prepare
derivative works, and perform publicly and display publicly.

                  DISCLAIMER

This computer code material was prepared, in part, as an account of work
sponsored by an agency of the United States Government.  Neither the United
States, nor the University of Chicago, nor any of their employees, makes any
warranty express or implied, or assumes any legal liability or responsibility
for the accuracy, completeness, or usefulness of any information, apparatus,
product, or process disclosed, or represents that its use would not infringe
privately owned rights.

 ***************************************************************************/

#include "comm_bench.h"

int main(int argc, char **argv)
{
#ifndef HAVE_BGP_PERSONALITY
    printf("BlueGene/P only!!!!!!!!!\n");
    return(911);
#else
    int desired = MPI_THREAD_MULTIPLE;
    int provided;
    MPI_Init_thread(&argc, &argv, desired, &provided);

    int me;
    int nproc;
    MPI_Comm_rank(MPI_COMM_WORLD,&me);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    //printf("%d: Hello world!\n",me);

    if ( me == 0 )
    {
        switch (provided)
        {
            case MPI_THREAD_MULTIPLE:
                printf("%d: provided = MPI_THREAD_MULTIPLE\n",me);
                break;

            case MPI_THREAD_SERIALIZED:
                printf("%d: provided = MPI_THREAD_SERIALIZED\n",me);
                break;

            case MPI_THREAD_FUNNELED:
                printf("%d: provided = MPI_THREAD_FUNNELED\n",me);
                break;

            case MPI_THREAD_SINGLE:
                printf("%d: provided = MPI_THREAD_SINGLE\n",me);
                break;

            default:
                printf("%d: MPI_Init_thread returned an invalid value of <provided>.\n",me);
                return(provided);

        }
    }

    if (me==0) printf("%d: before ARMCI_Init\n",me); fflush(stdout);
    ARMCI_Init();
    if (me==0) printf("%d: after  ARMCI_Init\n",me); fflush(stdout);
    int status;
    double t0,t1,t2,t3;
    double tt0,tt1,tt2,tt3;

    int a;
    if (me==0) for (a=0;a<argc;a++) printf("argv[%1d] = %s\n",a,argv[a]);
    int bufSize = ( argc>1 ? atoi(argv[1]) : 1000000 );
    if (me==0) printf("%d: bufSize = %d doubles\n",me,bufSize);

    /* register remote pointers */
    double** addrVec1 = (double **) malloc( nproc * sizeof(void *) );
    double** addrVec2 = (double **) malloc( nproc * sizeof(void *) );
    ARMCI_Malloc( (void **) addrVec1, bufSize * sizeof(double) );
    ARMCI_Malloc( (void **) addrVec2, bufSize * sizeof(double) );
    MPI_Barrier(MPI_COMM_WORLD);

    double* b1 = (double*) ARMCI_Malloc_local( bufSize * sizeof(double) ); assert(b1!=NULL);
    double* b2 = (double*) ARMCI_Malloc_local( bufSize * sizeof(double) ); assert(b2!=NULL);

    int i;
    for (i=0;i<bufSize;i++) b1[i]=1.0*me;
    for (i=0;i<bufSize;i++) b2[i]=-1.0;

    status = ARMCI_Put(b1, addrVec1[me], bufSize*sizeof(double), me); assert(status==0);
    status = ARMCI_Put(b2, addrVec2[me], bufSize*sizeof(double), me); assert(status==0);
    ARMCI_Barrier();

    _BGP_Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    uint32_t xSize = BGP_Personality_xSize(&pers);
    uint32_t ySize = BGP_Personality_ySize(&pers);
    uint32_t zSize = BGP_Personality_zSize(&pers);

    uint32_t torusMe[4];
    assert(0==Kernel_Rank2Coord(me,&torusMe[0],&torusMe[1],&torusMe[2],&torusMe[3]));

    uint32_t xJump,yJump,zJump;
    uint32_t count;

    uint32_t torusTarget[4];
    uint32_t target;
    uint32_t numnodes; // dummy

    double bandwidth;

    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);
    if (me==0){
        printf("ARMCI_Get performance test on %d nodes for %d doubles\n",nproc,bufSize);
        printf("iter  jump  (x,y,z)  host  (x,y,z) target (x,y,z)   local (s)     total (s)    effective BW (MB/s)\n");
        printf("===================================================================================================\n");
    }
    fflush(stdout);
    count = 0;
    for (xJump=0;xJump<xSize;xJump++){
        torusTarget[0] = (torusMe[0]+xJump)%xSize;
        for (yJump=0;yJump<=xJump;yJump++){
            torusTarget[1] = (torusMe[1]+yJump)%ySize;
            for (zJump=0;zJump<=yJump;zJump++){
                count++;
                MPI_Barrier(MPI_COMM_WORLD);
                fflush(stdout);
                torusTarget[2] = (torusMe[2]+zJump)%zSize;
                torusTarget[3] = torusMe[3];
                assert(0==Kernel_Coord2Rank(torusTarget[0],torusTarget[1],torusTarget[2],torusTarget[3],&target,&numnodes));
                MPI_Barrier(MPI_COMM_WORLD);
                t0 = MPI_Wtime();
                status = ARMCI_Get(addrVec1[target], b2, bufSize*sizeof(double), target); assert(status==0);
                t1 = MPI_Wtime();
                ARMCI_Fence(target);
                t2 = MPI_Wtime();
                fflush(stdout);
                for (i=0;i<bufSize;i++) assert( b2[i]==(1.0*target) );
                bandwidth = 1.0*bufSize*sizeof(double);
                bandwidth /= (t2-t0);
                bandwidth /= (1024*1024);
                printf("%4d (%2d,%2d,%2d) (%2d,%2d,%2d) (%2d,%2d,%2d)   %9.6f     %9.6f        %9.3f\n",
                       count,
                       abs(torusMe[0]-torusTarget[0]),abs(torusMe[1]-torusTarget[1]),abs(torusMe[2]-torusTarget[2]),
                       torusMe[0],torusMe[1],torusMe[2],
                       torusTarget[0],torusTarget[1],torusTarget[2],
                       t1-t0,t2-t0,bandwidth);
                fflush(stdout);
                MPI_Barrier(MPI_COMM_WORLD);
                if (me==0) printf("===================================================================================================\n");
                fflush(stdout);
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    status = ARMCI_Free_local(b2); assert(status==0);
    status = ARMCI_Free_local(b1); assert(status==0);

    status = ARMCI_Free(addrVec1[me]); assert(status==0);
    status = ARMCI_Free(addrVec2[me]); assert(status==0);

    MPI_Barrier(MPI_COMM_WORLD);

    if (me==0) printf("%d: ARMCI_Finalize\n",me);
    ARMCI_Finalize();

    if (me==0) printf("%d: MPI_Finalize\n",me);
    MPI_Finalize();

    return(0);
#endif // HAVE_BGP_PERSONALITY
}



