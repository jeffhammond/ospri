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
static volatile int thread_active[MAX_POSIX_THREADS];

static int size, rank;
static int num_pthreads;

void* foo(void* dummy)
{
    int i, my_pth = -1;
    pthread_t my_pthread = pthread_self();

    for (i=0 ; i<num_pthreads ; i++)
        if (my_pthread==thread_pool[i]) my_pth = i;
    
    fprintf(stdout, "hello from pthread %d on rank %d!\n" , my_pth, rank );
    fflush(stdout);

    while (thread_active[i]);

    fprintf(stdout, "bye from pthread %d on rank %d!\n" , my_pth, rank );
    fflush(stdout);

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
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);
 
    /********************************************************************/

    num_pthreads = ( argc>1 ? atoi(argv[1]) : MAX_POSIX_THREADS );

    fprintf(stdout,"rank %d creating %d threads\n", rank, num_pthreads);
    for (i=0 ; i<num_pthreads ; i++){
        thread_active[i] = 1;
        rc = pthread_create(&thread_pool[i], NULL, foo, NULL);
        assert(rc==0);
    }
    fprintf(stdout,"rank %d created %d threads\n", rank, num_pthreads);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    /********************************************************************/

    int buf_size = ( argc>2 ? atoi(argv[2]) : 16000 );

    /* allocate RMA buffers */
    double * wbuf;
    double * gbuf;
    double * pbuf;
    double * abuf;
    rc = MPI_Alloc_mem(buf_size * sizeof(double), MPI_INFO_NULL, &wbuf);
    rc = MPI_Alloc_mem(buf_size * sizeof(double), MPI_INFO_NULL, &gbuf);
    rc = MPI_Alloc_mem(buf_size * sizeof(double), MPI_INFO_NULL, &pbuf);
    rc = MPI_Alloc_mem(buf_size * sizeof(double), MPI_INFO_NULL, &abuf);

    /* initialize buffers */
    for (i=0;i<buf_size;i++) wbuf[i]=-1.0;
    for (i=0;i<buf_size;i++) pbuf[i]=3000.0;
    for (i=0;i<buf_size;i++) gbuf[i]=0.0;
    for (i=0;i<buf_size;i++) abuf[i]=700.0;

    MPI_Barrier(MPI_COMM_WORLD);

    /* register remote pointers */
    MPI_Win win;
    rc = MPI_Win_create(wbuf, buf_size * sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    /********************************************************************/

    if ( rank==0 )
    {
        int t;
        double t0, t1, t2, t3;

        printf("MPI_Put+Acc+Get performance test for buffer size = %d doubles\n",buf_size);
        printf("%10s %10s %10s %10s\n","t","put (us)","acc (us)","get (us)");
        printf("===========================================================\n");
        fflush(stdout);

        sleep(1);

        for (t=1;t<size;t++){

            t0 = MPI_Wtime();

            rc = MPI_Win_lock(MPI_LOCK_SHARED, t, MPI_MODE_NOCHECK, win);
            rc = MPI_Put(pbuf, buf_size, MPI_DOUBLE, t, 0, buf_size, MPI_DOUBLE, win);
            rc = MPI_Win_unlock(t, win);

            t1 = MPI_Wtime();

            rc = MPI_Win_lock(MPI_LOCK_SHARED, t, MPI_MODE_NOCHECK, win);
            rc = MPI_Accumulate(pbuf, buf_size, MPI_DOUBLE, t, 0, buf_size, MPI_DOUBLE, MPI_SUM, win);
            rc = MPI_Win_unlock(t, win);

            t2 = MPI_Wtime();

            rc = MPI_Win_lock(MPI_LOCK_SHARED, t, MPI_MODE_NOCHECK, win);
            rc = MPI_Get(gbuf, buf_size, MPI_DOUBLE, t, 0, buf_size, MPI_DOUBLE, win);
            rc = MPI_Win_unlock(t, win);

            t3 = MPI_Wtime();

            for (i=0;i<buf_size;i++) assert( gbuf[i]==3700.0 );
            
            printf("%10d %10lf %10lf %10lf\n",t,t1-t0,t2-t1,t3-t2);
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

    fprintf(stdout,"rank %d joining %d threads\n", rank, num_pthreads);
    for (i=0 ; i<num_pthreads ; i++){
        thread_active[i] = 0;
        rc = pthread_join(thread_pool[i],NULL);
        assert(rc==0);
    }
    fprintf(stdout,"rank %d joined %d threads\n", rank, num_pthreads);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    /********************************************************************/

    rc = MPI_Win_free(&win);
    rc = MPI_Free_mem(wbuf);
    rc = MPI_Free_mem(abuf);
    rc = MPI_Free_mem(gbuf);
    rc = MPI_Free_mem(pbuf);

    /********************************************************************/

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
