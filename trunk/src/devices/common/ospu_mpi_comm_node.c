#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int OSPU_Comm_split_node(MPI_Comm oldcomm, MPI_Comm * newcomm)
{
    int rc = MPI_SUCCESS;

#if MPI_VERSION >= 3

    rc = MPI_Comm_split_type(oldcomm, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, newcomm);
    if (rc!=MPI_SUCCESS) return rc;

#elif defined(MPICH2) && (MPICH2_NUMVERSION>10500000)

    rc = MPIX_Comm_split_type(oldcomm, MPIX_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, newcomm);
    if (rc!=MPI_SUCCESS) return rc;

#else

    /* This code was authored by Jim Dinan */

    char my_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm node_comm = MPI_COMM_NULL;
    MPI_Comm parent_comm;
    int len;

    /* Dup so we don't leak communicators */
    rc = MPI_Comm_dup(oldcomm, &parent_comm);
    if (rc!=MPI_SUCCESS) return rc;

    rc = MPI_Get_processor_name(my_name, &len);
    if (rc!=MPI_SUCCESS) return rc;

    while (node_comm == MPI_COMM_NULL)
    {
        char root_name[MPI_MAX_PROCESSOR_NAME];
        int  rank;
        MPI_Comm old_parent;

        rc = MPI_Comm_rank(parent_comm, &rank);
        if (rc!=MPI_SUCCESS) return rc;

        if (rank == 0)
        {
            rc = MPI_Bcast(my_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, parent_comm);
            if (rc!=MPI_SUCCESS) return rc;
            strncpy(root_name, my_name, MPI_MAX_PROCESSOR_NAME);
        } 
        else 
        {
            rc = MPI_Bcast(root_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, parent_comm);
            if (rc!=MPI_SUCCESS) return rc;
        }

        old_parent = parent_comm;

        if (strncmp(my_name, root_name, MPI_MAX_PROCESSOR_NAME) == 0)
        {
            /* My group splits off, I'm done after this */
            rc = MPI_Comm_split(parent_comm, 1, rank, &node_comm);
            if (rc!=MPI_SUCCESS) return rc;
        }
        else
        {
            /* My group keeps going, separate from the others */
            rc = MPI_Comm_split(parent_comm, 0, rank, &parent_comm);
            if (rc!=MPI_SUCCESS) return rc;
        }

        /* Old parent is no longer needed */
        rc = MPI_Comm_free(&old_parent);
        if (rc!=MPI_SUCCESS) return rc;
    }

    *new_comm = node_comm;

#endif

    return rc = MPI_SUCCESS;
}

