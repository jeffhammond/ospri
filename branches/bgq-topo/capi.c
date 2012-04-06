#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <mpi.h>

#include "q5d.h"

int main(int argc, char* argv[])
{
    int rank, size;

    int32_t coords[6];

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Q5D_Init();

    if (rank==0)
    {
        printf("%d: Q5D_Total_nodes = %d, Q5D_Total_procs = %d \n", rank, Q5D_Total_nodes(), Q5D_Total_procs() );

        Q5D_Partition_size(coords);
        printf("%d: Q5D_Torus_size = %d %d %d %d %d %d \n", rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

        Q5D_Partition_isTorus(coords);
        printf("%d: Q5D_Partition_isTorus = %d %d %d %d %d %d \n", rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

        Q5D_Job_size(coords);
        printf("%d: Q5D_Job_size = %d %d %d %d %d %d \n", rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

        Q5D_Job_isTorus(coords);
        printf("%d: Q5D_Job_isTorus = %d %d %d %d %d %d \n", rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
    }

    fflush(stdout);
    sleep(1);

    Q5D_Torus_coords(coords);
    printf("%d: Q5D_Node_rank() = %d, Q5D_Proc_rank = %d, Q5D_Core_id = %d, Q5D_Thread_id = %d, Q5D_Torus_coords = %d %d %d %d %d %d \n", rank, 
            Q5D_Node_rank(), Q5D_Proc_rank(), Q5D_Core_id(), Q5D_Thread_id(),
            coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

    fflush(stdout);
    sleep(1);

    MPI_Finalize();

    return 0;
}
