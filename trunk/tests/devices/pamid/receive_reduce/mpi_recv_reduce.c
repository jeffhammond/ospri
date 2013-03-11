#include <stdio.h>
#include <stdlib.h>
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

int MPIX_Recv_reduce(void * buf, int count, MPI_Datatype datatype, MPI_Op op, int source, int tag, MPI_Comm comm, MPI_Status * status)
{
    int rc = MPI_SUCCESS;

    rc = MPI_Probe(source, tag, comm, status);
    if (rc!=MPI_SUCCESS) return rc;

    /* acount = actual count */
    int acount;
    rc = MPI_Get_count(status, datatype, &acount); 
    if (rc!=MPI_SUCCESS) return rc;

    int size;
    rc = MPI_Type_size(datatype, &size);
    if (rc!=MPI_SUCCESS) return rc;

    /* rcount = recv/reduce count */
    /* buffer the smaller of the expected and actual count */
    int rcount = (acount>count) ? count : acount;

    if (op!=MPI_REPLACE)
    {
        void * tmp = malloc(rcount*size);
        if (tmp==NULL) return MPI_ERR_INTERN;

        rc = MPI_Recv(tmp, rcount, datatype, source, tag, comm, status);
        if (rc!=MPI_SUCCESS) return rc;

        /* It is unclear if this case needs to be supported.
         * MPI_NO_OP is probably not supported by MPI_Reduce_local.  */
#if (MPI_VERSION >= 3) 
        if (op!=MPI_NO_OP) 
#endif
        {
            rc = MPI_Reduce_local(tmp, buf, rcount, datatype, op);
            if (rc!=MPI_SUCCESS) return rc;
        }

        free(tmp);
    }
    else /* do not buffer MPI_REPLACE since it is equivalent to MPI_Recv */
    {
        rc = MPI_Recv(buf, rcount, datatype, source, tag, comm, status);
        if (rc!=MPI_SUCCESS) return rc;
    }

    return rc;
}

