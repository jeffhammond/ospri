#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <mpi.h>

#include "hpm.h"
#include "getticks.h"

int main(int argc, char* argv[])
{
    int rc;

    int desired = MPI_THREAD_FUNNELED;
    int provided;
    rc = MPI_Init_thread( &argc , &argv , desired , &provided ); assert( rc == MPI_SUCCESS );

    int rank, size;
    rc = MPI_Comm_rank( MPI_COMM_WORLD , &rank ); assert( rc == MPI_SUCCESS );
    rc = MPI_Comm_size( MPI_COMM_WORLD , &size ); assert( rc == MPI_SUCCESS );


    rc = MPI_Finalize();
    assert( rc == MPI_SUCCESS );

    return 0;
}
