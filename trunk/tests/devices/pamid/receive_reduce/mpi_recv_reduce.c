#include <mpi.h>

/*
 * int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
 * int MPI_Get_count( MPI_Status *status,  MPI_Datatype datatype, int *count );
 * int MPI_Type_size(MPI_Datatype datatype, int *size);
 * int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status); 
 * int MPI_Reduce_local(void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype, MPI_Op op); 
 *
 * typedef struct MPI_Status {
 *     int MPI_SOURCE;
 *     int MPI_TAG;
 *     int MPI_ERROR;
 *     MPI_Count count;
 *     int cancelled;
 * } MPI_Status;
 *
 */

int MPIX_Recv_reduce(void * buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status * status)
{
    int rc = MPI_SUCCESS;
    MPI_Status status;

    rc = MPI_Probe(source, tag, comm, &status);
    if (rc!=MPI_SUCCESS) return rc;

    int count;
    rc = MPI_Get_count(&status, datatype, &count ); 
    if (rc!=MPI_SUCCESS) return rc;

    int size;

    void * tmp = safemalloc(

    rc = MPI_Recv(buf, count, datatype, source, tag, comm, status);

    return rc;
}

