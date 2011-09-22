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
    int i, my_pth = -1;
    pthread_t my_pthread = pthread_self();

    for (i=0 ; i<num_pthreads ; i++)
        if (my_pthread==thread_pool[i]) my_pth = i;
    
    fprintf(stdout, "hello from pthread %d on rank %d!\n" , my_pth, rank );
    fflush(stdout);

    sleep(NAPTIME);

    fprintf(stdout, "bye from pthread %d on rank %d!\n" , my_pth, rank );
    fflush(stdout);

    sleep(NAPTIME);

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int i, rc;
    int provided;
 
    MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&provided);
    assert( provided == MPI_THREAD_MULTIPLE );
 
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
 
    printf("Hello from %d of %d processors\n", rank, size);
    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);
    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);
 
    num_pthreads = ( argc>1 ? atoi(argv[1]) : MAX_POSIX_THREADS );

    fprintf(stdout,"rank %d creating %d threads\n", rank, num_pthreads);
    for (i=0 ; i<num_pthreads ; i++){
        rc = pthread_create(&thread_pool[i], NULL, foo, NULL);
        assert(rc==0);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);
    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    for (i=0 ; i<num_pthreads ; i++){
        rc = pthread_join(thread_pool[i],NULL);
        assert(rc==0);
    }
    fprintf(stdout,"rank %d joined %d threads\n", rank, num_pthreads);

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);
    sleep(1);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
