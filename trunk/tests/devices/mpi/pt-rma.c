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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SINGLE, &provided);
    //assert(provided==MPI_THREAD_SINGLE);

    int me;
    int nproc;
    MPI_Comm_rank(MPI_COMM_WORLD,&me);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    int status;
    double t0,t1;

    int bufSize = ( argc>1 ? atoi(argv[1]) : 1000000 );
    if (me==0) printf("%d: bufSize = %d doubles\n",me,bufSize);

    /* allocate RMA buffers for windows */
    double* m1;
    double* m2;
    status = MPI_Alloc_mem(bufSize * sizeof(double), MPI_INFO_NULL, &m1);
    status = MPI_Alloc_mem(bufSize * sizeof(double), MPI_INFO_NULL, &m2);

    /* register remote pointers */
    MPI_Win w1;
    MPI_Win w2;
    status = MPI_Win_create(m1, bufSize * sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &w1);
    status = MPI_Win_create(m2, bufSize * sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &w2);
    MPI_Barrier(MPI_COMM_WORLD);

    /* allocate RMA buffers */
    double* b1;
    double* b2;
    status = MPI_Alloc_mem(bufSize * sizeof(double), MPI_INFO_NULL, &b1);
    status = MPI_Alloc_mem(bufSize * sizeof(double), MPI_INFO_NULL, &b2);

    /* initialize buffers */
    int i;
    for (i=0;i<bufSize;i++) b1[i]=1.0*me;
    for (i=0;i<bufSize;i++) b2[i]=-1.0;

    status = MPI_Win_fence( MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE , w1 );
    status = MPI_Win_fence( MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE , w2);
    status = MPI_Put(b1, bufSize, MPI_DOUBLE, me, 0, bufSize, MPI_DOUBLE, w1);
    status = MPI_Put(b2, bufSize, MPI_DOUBLE, me, 0, bufSize, MPI_DOUBLE, w2);
    status = MPI_Win_fence( MPI_MODE_NOSTORE , w1);
    status = MPI_Win_fence( MPI_MODE_NOSTORE , w2);

    int target;
    int j;
    double dt=0.0,bw=0.0;
    MPI_Barrier(MPI_COMM_WORLD);
    if (me==0){
        printf("MPI_Get performance test for buffer size = %d doubles\n",bufSize);
        printf("  jump    host   target       get (s)       BW (MB/s)\n");
        printf("===========================================================\n");
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    for (j=0;j<nproc;j++){
        target = (me+j) % nproc;
        MPI_Barrier(MPI_COMM_WORLD);
        t0 = MPI_Wtime();
        status = MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, w1);
        status = MPI_Get(b2, bufSize, MPI_DOUBLE, target, 0, bufSize, MPI_DOUBLE, w1);
        status = MPI_Win_unlock(target, w1);
        t1 = MPI_Wtime();
        for (i=0;i<bufSize;i++) assert( b2[i]==(1.0*target) );
        bw = (double)bufSize*sizeof(double)*(1e-6)/(t1-t0);
        printf("%4d     %4d     %4d       %9.6f     %9.3f\n",j,me,target,dt,bw);
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    status = MPI_Win_free(&w2);
    status = MPI_Win_free(&w1);

    status = MPI_Free_mem(b2);
    status = MPI_Free_mem(b1);

    status = MPI_Free_mem(m2);
    status = MPI_Free_mem(m1);

    MPI_Barrier(MPI_COMM_WORLD);

    if (me==0) printf("%d: MPI_Finalize\n",me);
    MPI_Finalize();

    return(0);
}



