#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

#include "q5d.h"

#define ITERATIONS 30

int main(int argc, char* argv[])
{
	int my_rank, world_size, buf_size, dest, tag;
	double start, end;
	void *buf = NULL;
	tag = 0;
	buf_size = 8*1024*1024;

	int32_t coords[6];

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	Q5D_Init();

	if (my_rank==0)
	{
		printf("%d: Q5D_Total_nodes = %d, Q5D_Total_procs = %d \n", my_rank, Q5D_Total_nodes(), Q5D_Total_procs() );

		Q5D_Partition_size(coords);
		printf("%d: Q5D_Torus_size = %d %d %d %d %d %d \n", my_rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

		Q5D_Partition_isTorus(coords);
		printf("%d: Q5D_Partition_isTorus = %d %d %d %d %d %d \n", my_rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

		Q5D_Job_size(coords);
		printf("%d: Q5D_Job_size = %d %d %d %d %d %d \n", my_rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

		Q5D_Job_isTorus(coords);
		printf("%d: Q5D_Job_isTorus = %d %d %d %d %d %d \n", my_rank, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
	}

	fflush(stdout);
	sleep(1);

	Q5D_Torus_coords(coords);
	printf("%d: Q5D_Node_rank() = %d, Q5D_Proc_rank = %d, Q5D_Core_id = %d, Q5D_Thread_id = %d, Q5D_Torus_coords = %d %d %d %d %d %d \n", my_rank, 
			Q5D_Node_rank(), Q5D_Proc_rank(), Q5D_Core_id(), Q5D_Thread_id(),
			coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);

	fflush(stdout);
	sleep(1);
	//Initialize the buffer to send/receive data
	buf = malloc(buf_size);
	int i = 0;
	if(my_rank == 0)
		printf("Size(bytes)\tLatency(usec)\tBandwidth(MB/s)\n");
	while(buf_size > 0) {
		//Start counting time
		if(my_rank == 0)
			start = MPI_Wtime();

		if(my_rank ==0) {
			//Send data
			for(i = 0; i< ITERATIONS; i++) {
				dest = 2;//my_rank + world_size/2;
				MPI_Send(buf, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
				//Receive data
				MPI_Recv(buf, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else if(my_rank ==2) {
			for(i = 0; i < ITERATIONS; i++) {
				dest = 0;//my_rank - world_size/2;
				//Receive data
				MPI_Recv(buf, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//Send data
				MPI_Send(buf, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
			}
		}

		//End of the communication. Get the time.
		if(my_rank == 0) {
			end = MPI_Wtime();
			double latency = (end-start)*1000000/(ITERATIONS);
			double bandwidth = ((double)buf_size*ITERATIONS)*2/((end-start)*1024*1024);
			printf("%d\t\t%f\t\t%f\n", buf_size, latency, bandwidth);
		}

		buf_size = buf_size/2;
	}
	free(buf);


	MPI_Finalize();

	return 0;
}
