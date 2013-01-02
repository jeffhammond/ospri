#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>

#include <mpi.h>

#define DEBUG

pthread_t Progress_thread;
 
static void * Progress_function(void * dummy)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	while (1)
	{
#ifdef DEBUG
        printf("%d: Progress \n", rank);
#endif
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//usleep(500);
	}

	return NULL;
}

int main(int argc, char * argv[])
{
    int rc;
    int rank, size;
    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided!=MPI_THREAD_MULTIPLE) MPI_Abort(MPI_COMM_WORLD, 1);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    rc = pthread_create(&Progress_thread, NULL, &Progress_function, NULL);
    if (rc!=0) MPI_Abort(MPI_COMM_WORLD, rc);

    MPI_Barrier(MPI_COMM_WORLD);

    printf("before pthread shutdown \n");
    fflush(stdout);

    MPI_Barrier(MPI_COMM_WORLD);

#if 0
    rc = pthread_cancel(Progress_thread);
    if (rc!=0) MPI_Abort(MPI_COMM_WORLD, rc);

    void * rv;
    rc = pthread_join(Progress_thread, &rv);
    if (rc!=0) MPI_Abort(MPI_COMM_WORLD, rc);

    printf("after pthread shutdown \n");
    fflush(stdout);

    MPI_Barrier(MPI_COMM_WORLD);
#endif

    printf("before MPI Abort/Finalize \n");
    fflush(stdout);

    //MPI_Abort(MPI_COMM_SELF, 2);
    MPI_Finalize();

    printf("after MPI Abort/Finalize \n");
    fflush(stdout);

    return 0;
} 
