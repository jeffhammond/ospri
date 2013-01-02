#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>

#include <mpi.h>

#include "safemalloc.h"

//#define DEBUG

pthread_t Progress_thread;
MPI_Comm MSG_COMM_WORLD;
 
#define MSG_INFO_TAG 100

#define MSG_GET 1
#define MSG_PUT 2
#define MSG_ACC 4
#define MSG_RMW 8

#define MSG_GET_TAG 100+MSG_GET
#define MSG_PUT_TAG 100+MSG_PUT
#define MSG_ACC_TAG 100+MSG_ACC
#define MSG_RMW_TAG 100+MSG_RMW

typedef struct
{
    int type;
    void * address;
    size_t length;
}
msg_info_t;

typedef struct
{
    MPI_Comm comm;
    void ** base;
}
msg_window_t;

void Poll(void)
{
    msg_info_t info;
    MPI_Status status;
    MPI_Recv(&info, sizeof(msg_info_t), MPI_BYTE, MPI_ANY_SOURCE, MSG_INFO_TAG, MSG_COMM_WORLD, &status);

    int source = status.MPI_SOURCE;

    if (info.type == MSG_GET)
    {
#ifdef DEBUG
        printf("MSG_GET \n");
#endif
        if (info.length<INT_MAX)
            MPI_Send(info.address, info.length, MPI_BYTE, source, MSG_GET_TAG, MSG_COMM_WORLD);
        else /* TODO: need to implement long-message protocol */
            MPI_Abort(MSG_COMM_WORLD, 1);
    }
    else if (info.type == MSG_PUT)
    {
#ifdef DEBUG
        printf("MSG_PUT \n");
#endif
        if (info.length<INT_MAX)
        {
            MPI_Status status;
            MPI_Recv(info.address, info.length, MPI_BYTE, source, MSG_PUT_TAG, MSG_COMM_WORLD, &status);
        }
        else /* TODO: need to implement long-message protocol */
            MPI_Abort(MSG_COMM_WORLD, 1);
    }
    else if (info.type == MSG_ACC)
    {
#ifdef DEBUG
        printf("MSG_ACC \n");
#endif
        MPI_Abort(MSG_COMM_WORLD, 1);
    }
    else if (info.type == MSG_RMW)
    {
#ifdef DEBUG
        printf("MSG_RMW \n");
#endif
        MPI_Abort(MSG_COMM_WORLD, 1);
    }
    return;
}

static void * Progress_function(void * dummy)
{
    int rank;
    MPI_Comm_rank(MSG_COMM_WORLD, &rank);

	while (1)
	{
#ifdef DEBUG
        printf("%d: Progress \n", rank);
#endif
        Poll();
		//usleep(500);
	}

	return NULL;
}

void MSG_Win_get(int target, msg_window_t * win, size_t offset, size_t length, void * buffer)
{
    msg_info_t info;

    info.type = MSG_GET;
    info.address = win->base[target]+offset;
    if (length<INT_MAX)
        info.length = length;
    else
        MPI_Abort(MSG_COMM_WORLD, 1);

#ifdef DEBUG
    printf("MSG_Win_get win->base[%d]=%p address=%p length=%ld\n", target, win->base[target], info.address, info.length);
    fflush(stdout);
#endif

    MPI_Ssend(&info, sizeof(msg_info_t), MPI_BYTE, target, MSG_INFO_TAG, MSG_COMM_WORLD);

    if (length<INT_MAX)
    {
        MPI_Recv(buffer, length, MPI_BYTE, target, MSG_GET_TAG, MSG_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
        /* TODO: need to implement long-message protocol */
        MPI_Abort(MSG_COMM_WORLD, 1);
    }

    return;
}

void MSG_Win_put(int target, msg_window_t * win, size_t offset, size_t length, void * buffer)
{
    msg_info_t info;

    info.type = MSG_PUT;
    info.address = win->base[target]+offset;
    if (length<INT_MAX)
        info.length = length;
    else
        MPI_Abort(MSG_COMM_WORLD, 1);

#ifdef DEBUG
    printf("MSG_Win_put win->base[%d]=%p address=%p length=%ld\n", target, win->base[target], info.address, info.length);
    fflush(stdout);
#endif

    MPI_Ssend(&info, sizeof(msg_info_t), MPI_BYTE, target, MSG_INFO_TAG, MSG_COMM_WORLD);

    if (length<INT_MAX)
    {
        MPI_Send(buffer, length, MPI_BYTE, target, MSG_PUT_TAG, MSG_COMM_WORLD);
    }
    else
    {
        /* TODO: need to implement long-message protocol */
        MPI_Abort(MSG_COMM_WORLD, 1);
    }

    return;
}

void MSG_Win_allocate(MPI_Comm comm, size_t bytes, msg_window_t * win)
{
    MPI_Comm_dup(comm, &(win->comm));

    int rank, size;
    MPI_Comm_rank(win->comm, &rank);
    MPI_Comm_size(win->comm, &size);

    win->base = safemalloc( size * sizeof(void *) );

    void * my_base = safemalloc(bytes);

    MPI_Allgather(&my_base, sizeof(void *), MPI_BYTE, win->base, sizeof(void *), MPI_BYTE, comm);

#ifdef DEBUG
    printf("%d: win->base = %p \n", rank, win->base);
    for (int i=0; i<size; i++)
        printf("%d: win->base[%d] = %p \n", rank, i, win->base[i]);

    printf("MSG_Win_allocate finished\n");
    fflush(stdout);
#endif

    return;
}

void MSG_Win_deallocate(msg_window_t * win)
{
    MPI_Barrier(win->comm);

    int rank;
    MPI_Comm_rank(win->comm, &rank);

    free(win->base[rank]);
    free(win->base);

    MPI_Comm_free(&(win->comm));

#ifdef DEBUG
    printf("MSG_Win_deallocate finished\n");
    fflush(stdout);
#endif

    return;   
}

int main(int argc, char * argv[])
{
    int rank, size;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided!=MPI_THREAD_MULTIPLE) 
        MPI_Abort(MPI_COMM_WORLD, 1);

    MPI_Comm_dup(MPI_COMM_WORLD, &MSG_COMM_WORLD);

    MPI_Comm_rank(MSG_COMM_WORLD, &rank);
    MPI_Comm_size(MSG_COMM_WORLD, &size);

    int rc;

    rc = pthread_create(&Progress_thread, NULL, &Progress_function, NULL);
    if (rc!=0) 
        MPI_Abort(MSG_COMM_WORLD, rc);

    MPI_Barrier(MSG_COMM_WORLD);

    int bigcount = 1024*1024;
    msg_window_t win;
    MSG_Win_allocate(MSG_COMM_WORLD, bigcount, &win);

    if (rank==0)
    {
        int smallcount = 1024;

        void * in = safemalloc(smallcount);
        void * out = safemalloc(smallcount);
        
        memset(in, '\1', smallcount);
        memset(out, '\0', smallcount);

        MSG_Win_put(size>1 ? size-1 : 0, &win, 0, smallcount, in);
        MSG_Win_get(size>1 ? size-1 : 0, &win, 0, smallcount, out);

        int rc = memcmp(in, out, smallcount);

        if (rc!=0)
            printf("FAIL! \n");
        else
            printf("WIN! \n");
        fflush(stdout);

        free(out);
        free(in);
    }

    MSG_Win_deallocate(&win);

    printf("test done except for pthread shutdown \n");
    fflush(stdout);

    MPI_Barrier(MSG_COMM_WORLD);

    rc = pthread_cancel(Progress_thread);
    if (rc!=0) 
        MPI_Abort(MSG_COMM_WORLD, rc);

    void * rv;
    rc = pthread_join(Progress_thread, &rv);
    if (rc!=0) 
        MPI_Abort(MSG_COMM_WORLD, rc);

    printf("all done \n");
    fflush(stdout);

    MPI_Finalize();

    return 0;
} 
