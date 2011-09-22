#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>

#define NAPTIME 3

#define MAX_POSIX_THREADS 64

static pthread_t thread_pool[MAX_POSIX_THREADS];

static int size, rank;
static int num_pthreads;

void* foo(void* dummy)
{
    int i, my_pth;
    pthread_t my_pthread = pthread_self();

    for (i=0 ; i<num_pthreads ; i++)
        if (my_pthread==thread_pool[i]) my_pth = i;
    
    //fprintf(stdout, "hello from pthread %d on rank %d!\n" , my_pth, rank );
    fflush(stdout);

    sleep(NAPTIME);

    //fprintf(stdout, "bye from pthread %d on rank %d!\n" , my_pth, rank );
    fflush(stdout);

    sleep(NAPTIME);

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int i, rc;
    int provided;
    int namelen;
    char procname[MPI_MAX_PROCESSOR_NAME];
 
    MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&provided);
    assert( provided == MPI_THREAD_MULTIPLE );
 
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
 
    MPI_Get_processor_name( procname, &namelen );
    printf( "Hello from rank %d (processor name = %s) of %d\n" , rank, procname, size );
    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);
    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);
 
    /********************************************************************/

    int status;
    double t0,t1;

    int bufSize = ( argc>2 ? atoi(argv[2]) : 1000000 );
    if (rank==0) printf("%d: bufSize = %d doubles\n",rank,bufSize);

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

    /* allocate RMA buffers */
    double* b1;
    double* b2;
    status = MPI_Alloc_mem(bufSize * sizeof(double), MPI_INFO_NULL, &b1);
    status = MPI_Alloc_mem(bufSize * sizeof(double), MPI_INFO_NULL, &b2);

    /* initialize buffers */
    for (i=0;i<bufSize;i++) b1[i]=1.0*rank;
    for (i=0;i<bufSize;i++) b2[i]=-1.0;

    MPI_Barrier(MPI_COMM_WORLD);

    /********************************************************************/

    status = MPI_Win_fence( MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE , w1 );
    status = MPI_Win_fence( MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE , w2);
    status = MPI_Put(b1, bufSize, MPI_DOUBLE, rank, 0, bufSize, MPI_DOUBLE, w1);
    status = MPI_Put(b2, bufSize, MPI_DOUBLE, rank, 0, bufSize, MPI_DOUBLE, w2);
    status = MPI_Win_fence( MPI_MODE_NOSTORE , w1);
    status = MPI_Win_fence( MPI_MODE_NOSTORE , w2);

    /********************************************************************/

    if ( rank==0 )
    {

        int target;
        int j;
        double dt=0.0,bw=0.0;

        printf("MPI_Get performance test for buffer size = %d doubles\n",bufSize);
        printf("  jump    host   target       get (s)       BW (MB/s)\n");
        printf("===========================================================\n");
        fflush(stdout);

        sleep(1);
        for (j=1;j<size;j++){
            target = (rank+j) % size;
            t0 = MPI_Wtime();
            status = MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, w1);
            status = MPI_Get(b2, bufSize, MPI_DOUBLE, target, 0, bufSize, MPI_DOUBLE, w1);
            status = MPI_Win_unlock(target, w1);
            t1 = MPI_Wtime();
            for (i=0;i<bufSize;i++) assert( b2[i]==(1.0*target) );
            bw = (double)bufSize*sizeof(double)*(1e-6)/(t1-t0);
            printf("%4d     %4d     %4d       %9.6f     %9.3f\n",j,rank,target,dt,bw);
            fflush(stdout);
        }
        sleep(1);
        for (j=size;j>0;j--){
            target = (rank+j) % size;
            t0 = MPI_Wtime();
            status = MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, w1);
            status = MPI_Get(b2, bufSize, MPI_DOUBLE, target, 0, bufSize, MPI_DOUBLE, w1);
            status = MPI_Win_unlock(target, w1);
            t1 = MPI_Wtime();
            for (i=0;i<bufSize;i++) assert( b2[i]==(1.0*target) );
            bw = (double)bufSize*sizeof(double)*(1e-6)/(t1-t0);
            printf("%4d     %4d     %4d       %9.6f     %9.3f\n",j,rank,target,dt,bw);
            fflush(stdout);
        }
        sleep(1);
    }
    else
    {
        sleep(10);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /********************************************************************/

    num_pthreads = ( argc>1 ? atoi(argv[1]) : MAX_POSIX_THREADS );

    fprintf(stdout,"rank %d creating %d threads\n", rank, num_pthreads);
    for (i=0 ; i<num_pthreads ; i++){
        rc = pthread_create(&thread_pool[i], NULL, foo, NULL);
        assert(rc==0);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);

    /********************************************************************/

    status = MPI_Win_fence( MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE , w1 );
    status = MPI_Win_fence( MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE , w2);
    status = MPI_Put(b1, bufSize, MPI_DOUBLE, rank, 0, bufSize, MPI_DOUBLE, w1);
    status = MPI_Put(b2, bufSize, MPI_DOUBLE, rank, 0, bufSize, MPI_DOUBLE, w2);
    status = MPI_Win_fence( MPI_MODE_NOSTORE , w1);
    status = MPI_Win_fence( MPI_MODE_NOSTORE , w2);

    /********************************************************************/

    if ( rank==0 )
    {
        int target;
        int j;
        double dt=0.0,bw=0.0;

        printf("MPI_Get performance test for buffer size = %d doubles\n",bufSize);
        printf("  jump    host   target       get (s)       BW (MB/s)\n");
        printf("===========================================================\n");
        fflush(stdout);

        sleep(1);
        for (j=1;j<size;j++){
            target = (rank+j) % size;
            t0 = MPI_Wtime();
            status = MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, w1);
            status = MPI_Get(b2, bufSize, MPI_DOUBLE, target, 0, bufSize, MPI_DOUBLE, w1);
            status = MPI_Win_unlock(target, w1);
            t1 = MPI_Wtime();
            for (i=0;i<bufSize;i++) assert( b2[i]==(1.0*target) );
            bw = (double)bufSize*sizeof(double)*(1e-6)/(t1-t0);
            printf("%4d     %4d     %4d       %9.6f     %9.3f\n",j,rank,target,dt,bw);
            fflush(stdout);
        }
        sleep(1);
        for (j=size;j>0;j--){
            target = (rank+j) % size;
            t0 = MPI_Wtime();
            status = MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, w1);
            status = MPI_Get(b2, bufSize, MPI_DOUBLE, target, 0, bufSize, MPI_DOUBLE, w1);
            status = MPI_Win_unlock(target, w1);
            t1 = MPI_Wtime();
            for (i=0;i<bufSize;i++) assert( b2[i]==(1.0*target) );
            bw = (double)bufSize*sizeof(double)*(1e-6)/(t1-t0);
            printf("%4d     %4d     %4d       %9.6f     %9.3f\n",j,rank,target,dt,bw);
            fflush(stdout);
        }
        sleep(1);
    }
    else
    {
        sleep(10);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /********************************************************************/

    for (i=0 ; i<num_pthreads ; i++){
        rc = pthread_join(thread_pool[i],NULL);
        assert(rc==0);
    }
    fprintf(stdout,"rank %d joined %d threads\n", rank, num_pthreads);

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);

    /********************************************************************/

    status = MPI_Win_free(&w2);
    status = MPI_Win_free(&w1);

    status = MPI_Free_mem(b2);
    status = MPI_Free_mem(b1);

    status = MPI_Free_mem(m2);
    status = MPI_Free_mem(m1);

    MPI_Barrier(MPI_COMM_WORLD);

    /********************************************************************/

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);
    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
