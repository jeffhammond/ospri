/*
 * This is a simple program to measure the time to transfer data between 2 nodes, using MPI_Send and MPI_Recv primitives.
 *
*/
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define ITERATIONS 30
#define MAX_DATA_TRANS 16*1024*1024
#define MIN_DATA_TRANS 1

int main(int argc, char** argv) {

	int my_rank, world_size, buf_size, dest, tag;
	double start, end;
	void *buf = NULL, *buf1 = NULL;

	tag = 0;
	buf_size = MIN_DATA_TRANS;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if(world_size < 2) {
		fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	//Initialize the buffer to send/receive data
	buf = malloc(MAX_DATA_TRANS);
	buf1 = malloc(MAX_DATA_TRANS);
	int i = 0;
	int gap = world_size/2;
	struct timeval t1, t2;
	if(my_rank == 0)
		printf("Size(bytes)\tLatency(usec)\tBandwidth(MB/s)\n");
	while(buf_size <= MAX_DATA_TRANS) {
		//Start counting time
		if(my_rank == 0) {
			start = MPI_Wtime();
			//gettimeofday(&t1, NULL);
		}

		if(my_rank %2 == 0) {
			//Send data
			for(i = 0; i< ITERATIONS; i++) {
				dest = my_rank + 1;
				MPI_Send(buf, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
				//memcpy(buf1, buf, buf_size);
				//Receive data
				MPI_Recv(buf1, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else  {
			for(i = 0; i < ITERATIONS; i++) {
				dest = my_rank -1;
				//Receive data
				MPI_Recv(buf, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//memcpy(buf1, buf, buf_size);
				//Send data
				MPI_Send(buf1, buf_size, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
			}
		}

		//End of the communication. Get the time.
		if(my_rank == 0) {
			end = MPI_Wtime();
			double latency = ((end-start)*1000000)/ITERATIONS;
			//gettimeofday(&t2, NULL);
			//double latency = ((t2.tv_sec * 1000000 + t2.tv_usec)  - (t1.tv_sec * 1000000 + t1.tv_usec))/ITERATIONS;
			double bandwidth = buf_size*1000000.0*2/(latency*1024*1024);
			printf("%d\t\t%f\t\t%f\n", buf_size, latency, bandwidth*2);
		}

		buf_size = buf_size*2;
	}
	free(buf);
	MPI_Finalize();
	return 0;
}
