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

#define MSG_FENCE     1
#define MSG_GET       2
#define MSG_PUT       4
#define MSG_ACC       8
#define MSG_RMW      16 
#define MSG_CHT_EXIT 32

#define MSG_FENCE_TAG 100+MSG_FENCE
#define MSG_GET_TAG   100+MSG_GET
#define MSG_PUT_TAG   100+MSG_PUT
#define MSG_ACC_TAG   100+MSG_ACC
#define MSG_RMW_TAG   100+MSG_RMW

#if defined(DEBUG) || 0
#  define MSG_MAX_COUNT 1024*1024
#else
#  define MSG_MAX_COUNT INT_MAX
#endif

typedef struct
{
    size_t       count; 
    MPI_Datatype dt;
    MPI_Op       op; /* only used for MSG_ACC */
}
msg_rma_info_t;

typedef struct
{
    size_t       count; 
    MPI_Datatype dt;
}
msg_rmw_info_t;

typedef struct
{
    int          type;
    void *       address;
    size_t       count; 
    MPI_Datatype dt;
    MPI_Op       op; /* only used for MSG_ACC */
}
msg_info_t;

typedef struct
{
    MPI_Comm comm;
    void ** base;
    void *  my_base;
}
msg_window_t;

void Poll(void)
{
    int rank;
    int type_size;

    msg_info_t info;
    MPI_Status status;
    MPI_Recv(&info, sizeof(msg_info_t), MPI_BYTE, MPI_ANY_SOURCE, MSG_INFO_TAG, MSG_COMM_WORLD, &status);
    int source = status.MPI_SOURCE;

    switch (info.type)
    {
        case MSG_FENCE:
#ifdef DEBUG
            printf("MSG_FENCE \n");
#endif
            MPI_Send(NULL, 0, MPI_BYTE, source, MSG_FENCE_TAG, MSG_COMM_WORLD);
            break;

        case MSG_GET:
#ifdef DEBUG
            printf("MSG_GET \n");
#endif
            if (info.count<MSG_MAX_COUNT)
            {
                int count = (int) info.count;
                MPI_Send(info.address, count, info.dt, source, MSG_GET_TAG, MSG_COMM_WORLD);
            }
            else /* TODO: need to implement long-message protocol */
                MPI_Abort(MSG_COMM_WORLD, 1);
            break;

        case MSG_PUT:
#ifdef DEBUG
            printf("MSG_PUT \n");
#endif
            if (info.count<MSG_MAX_COUNT)
            {
                int count = (int) info.count;
                MPI_Recv(info.address, count, info.dt, source, MSG_PUT_TAG, MSG_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else /* TODO: need to implement long-message protocol */
                MPI_Abort(MSG_COMM_WORLD, 1);

            break;

        case MSG_ACC:
#ifdef DEBUG
            printf("MSG_ACC \n");
#endif
            if (info.count<MSG_MAX_COUNT)
            {
                /* TODO: need to do pipelining to limit buffering */

                MPI_Type_size(info.dt, &type_size);
                void * temp = safemalloc(info.count*type_size);
#ifdef DEBUG
                printf("temp = %p \n", temp);
#endif

                int count = (int) info.count;
                MPI_Recv(temp, count, info.dt, source, MSG_ACC_TAG, MSG_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Reduce_local(temp, info.address, count, info.dt, info.op);

                free(temp);
            }
            else /* TODO: need to implement long-message protocol */
                MPI_Abort(MSG_COMM_WORLD, 1);

            break;

        case MSG_RMW:
#ifdef DEBUG
            printf("MSG_RMW \n");
#endif
            MPI_Comm_rank(MSG_COMM_WORLD, &rank);
            fprintf(stderr, "%d: MSG_RMW not supported yet \n", rank);
            MPI_Abort(MSG_COMM_WORLD, 1);
            break;

        case MSG_CHT_EXIT:
#ifdef DEBUG
            printf("MSG_CHT_EXIT \n");
#endif
            MPI_Comm_rank(MSG_COMM_WORLD, &rank);

            if (rank!=source)
            {
                fprintf(stderr, "%d: CHT received EXIT signal from rank %d \n", rank, source);
                MPI_Abort(MSG_COMM_WORLD, 1);
            }

            pthread_exit(NULL);

            break;

        default:
            MPI_Comm_rank(MSG_COMM_WORLD, &rank);
            fprintf(stderr, "%d: CHT received invalid MSG TAG (%d) \n", rank, info.type);
            MPI_Abort(MSG_COMM_WORLD, 1);
            break;
    }
    return;
}

static void * Progress_function(void * dummy)
{
    int rank;
    MPI_Comm_rank(MSG_COMM_WORLD, &rank);

	while (1)
	{
        Poll();
		//usleep(500);
	}

	return NULL;
}

void MSG_CHT_Exit(void)
{
    int rank;
    MPI_Comm_rank(MSG_COMM_WORLD, &rank);

    msg_info_t info;
    info.type = MSG_CHT_EXIT;
    MPI_Ssend(&info, sizeof(msg_info_t), MPI_BYTE, rank, MSG_INFO_TAG, MSG_COMM_WORLD);

    return;
}

void MSG_Win_fence(int target)
{
    msg_info_t info;

    info.type     = MSG_FENCE;

    MPI_Send(&info, sizeof(msg_info_t), MPI_BYTE, target, MSG_INFO_TAG, MSG_COMM_WORLD);

    MPI_Recv(NULL, 0, MPI_BYTE, target, MSG_FENCE_TAG, MSG_COMM_WORLD, MPI_STATUS_IGNORE);

    return;
}

void MSG_Win_get(int target, msg_window_t * win, size_t offset, size_t count, MPI_Datatype type, void * buffer)
{
    msg_info_t info;

    info.type     = MSG_GET;
    info.address  = win->base[target]+offset;
    info.count    = count;
    info.dt       = type;

#ifdef DEBUG
    printf("MSG_Win_get win->base[%d]=%p address=%p count=%ld\n", target, win->base[target], info.address, info.count);
    fflush(stdout);
#endif

    MPI_Send(&info, sizeof(msg_info_t), MPI_BYTE, target, MSG_INFO_TAG, MSG_COMM_WORLD);

    if (info.count<MSG_MAX_COUNT)
    {
        int count = (int) info.count;
        MPI_Recv(buffer, count, info.dt, target, MSG_GET_TAG, MSG_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
#ifdef NOT_DONE
        size_t num   = info.count/MSG_MAX_COUNT;
        size_t count = info.count/num;
        if (num>INT_MAX || count>INT_MAX) /* extremely unlikely unless MSG_MAX_COUNT is very small */
        {
            int rank;
            MPI_Comm_rank(MSG_COMM_WORLD, &rank);

            fprintf("%d: number of messages (%ld) or their count (%ld) is too large \n", rank, num, count);
            MPI_Abort(MSG_COMM_WORLD, 1);
        }

        int num_msg   = (int)num;
        int msg_count = (int)count;
        int rem_count = ;
#else
        /* TODO: need to implement long-message protocol */
        MPI_Abort(MSG_COMM_WORLD, 1);
#endif        
    }

    return;
}

void MSG_Win_put(int target, msg_window_t * win, size_t offset, size_t count, MPI_Datatype type, void * buffer)
{
    msg_info_t info;

    info.type     = MSG_PUT;
    info.address  = win->base[target]+offset;
    info.count    = count; 
    info.dt       = type;

#ifdef DEBUG
    printf("MSG_Win_put win->base[%d]=%p address=%p count=%ld\n", target, win->base[target], info.address, info.count);
    fflush(stdout);
#endif

    MPI_Send(&info, sizeof(msg_info_t), MPI_BYTE, target, MSG_INFO_TAG, MSG_COMM_WORLD);

    if (info.count<MSG_MAX_COUNT)
    {
        int count = (int) info.count;
        MPI_Send(buffer, count, info.dt, target, MSG_PUT_TAG, MSG_COMM_WORLD);
    }
    else
    {
        /* TODO: need to implement long-message protocol */
        MPI_Abort(MSG_COMM_WORLD, 1);
    }

    return;
}

void MSG_Win_acc(int target, msg_window_t * win, size_t offset, size_t count, MPI_Datatype type, MPI_Op op, void * buffer)
{
    msg_info_t info;

    info.type     = MSG_ACC;
    info.address  = win->base[target]+offset;
    info.count    = count; 
    info.dt       = type;
    info.op       = op;

#ifdef DEBUG
    printf("MSG_Win_acc win->base[%d]=%p address=%p count=%ld type=%d\n", target, win->base[target], info.address, info.count, info.dt);
    fflush(stdout);
#endif

    MPI_Send(&info, sizeof(msg_info_t), MPI_BYTE, target, MSG_INFO_TAG, MSG_COMM_WORLD);

    if (info.count<MSG_MAX_COUNT)
    {
        int count = (int) info.count;
        MPI_Send(buffer, count, info.dt, target, MSG_ACC_TAG, MSG_COMM_WORLD);
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

    win->base    = safemalloc( size * sizeof(void *) );
    win->my_base = safemalloc(bytes);

    MPI_Allgather(&(win->my_base), sizeof(void *), MPI_BYTE, win->base, sizeof(void *), MPI_BYTE, comm);

#ifdef DEBUG
    printf("%d: win->base = %p \n", rank, win->base);
    printf("%d: my_base = %p \n", rank, win->my_base);
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

    free(win->my_base);
    free(win->base);

    MPI_Comm_free(&(win->comm));

#ifdef DEBUG
    printf("%d: win->base = %p \n", rank, win->base);
    printf("%d: win->my_base = %p \n", rank, win->my_base);
    printf("%d: win->base[%d] = %p \n", rank, rank, win->base[rank]);

    printf("MSG_Win_deallocate finished\n");
    fflush(stdout);
#endif

    return;   
}

int main(int argc, char * argv[])
{
    int rc;
    int rank, size;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided!=MPI_THREAD_MULTIPLE) 
        MPI_Abort(MPI_COMM_WORLD, 1);

    MPI_Comm_dup(MPI_COMM_WORLD, &MSG_COMM_WORLD);

    MPI_Comm_rank(MSG_COMM_WORLD, &rank);
    MPI_Comm_size(MSG_COMM_WORLD, &size);

    rc = pthread_create(&Progress_thread, NULL, &Progress_function, NULL);
    if (rc!=0) 
        MPI_Abort(MSG_COMM_WORLD, rc);

    MPI_Barrier(MSG_COMM_WORLD);

    {
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

            MSG_Win_put(size>1 ? size-1 : 0, &win, 0, smallcount, MPI_BYTE, in);
            MSG_Win_fence(size>1 ? size-1 : 0);
            MSG_Win_get(size>1 ? size-1 : 0, &win, 0, smallcount, MPI_BYTE, out);

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
    }

    {
        int bigcount = 1024*1024;

        MPI_Op       op   = MPI_SUM;
        MPI_Datatype type = MPI_DOUBLE;

        int type_size;
        MPI_Type_size(type, &type_size);
        
        msg_window_t win;
        MSG_Win_allocate(MSG_COMM_WORLD, bigcount*type_size, &win);

        if (rank==0)
        {
            int smallcount = 1024;

            double * in = safemalloc(smallcount*type_size);
            double * out = safemalloc(smallcount*type_size);
            
            for (int i=0; i<smallcount; i++)
                in[i] = 0.0;

            printf("MSG_Win_put \n");
            fflush(stdout);
            MSG_Win_put(size>1 ? size-1 : 0, &win, 0, smallcount, type, in);
            MSG_Win_fence(size>1 ? size-1 : 0);

            for (int i=0; i<smallcount; i++)
                in[i] = 12.0;

            printf("MSG_Win_acc \n");
            fflush(stdout);
            MSG_Win_acc(size>1 ? size-1 : 0, &win, 0, smallcount, type, op, in);
            MSG_Win_fence(size>1 ? size-1 : 0);
            MSG_Win_acc(size>1 ? size-1 : 0, &win, 0, smallcount, type, op, in);
            MSG_Win_fence(size>1 ? size-1 : 0);
            MSG_Win_acc(size>1 ? size-1 : 0, &win, 0, smallcount, type, op, in);
            MSG_Win_fence(size>1 ? size-1 : 0);

            printf("MSG_Win_get \n");
            fflush(stdout);
            MSG_Win_get(size>1 ? size-1 : 0, &win, 0, smallcount, type, out);

            int errors = 0;
            for (int i=0; i<smallcount; i++)
#if defined(DEBUG) && 0
                printf("%d: out[%d] = %lf  in[%d] = %lf \n", rank, i, out[i], i, in[i]);
#else
                errors += (int)(out[i] != 3*in[i]);
#endif

            if (errors>0)
                printf("FAIL! \n");
            else
                printf("WIN! \n");
            fflush(stdout);

            free(out);
            free(in);
        }

        MSG_Win_deallocate(&win);
    }

    printf("test done except for pthread shutdown \n");
    fflush(stdout);

    MPI_Barrier(MSG_COMM_WORLD);

#if 1
    MSG_CHT_Exit();
#else
    rc = pthread_cancel(Progress_thread);
    if (rc!=0) 
        MPI_Abort(MSG_COMM_WORLD, rc);
#endif

    void * rv;
    rc = pthread_join(Progress_thread, &rv);
    if (rc!=0) 
        MPI_Abort(MSG_COMM_WORLD, rc);

    printf("all done \n");
    fflush(stdout);

    MPI_Comm_free(&MSG_COMM_WORLD);

    MPI_Finalize();

    return 0;
} 
