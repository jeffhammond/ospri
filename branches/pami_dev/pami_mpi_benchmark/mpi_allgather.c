/*
 * This is a simple program to measure the time to transfer data using MPI_Allgather primitives.
 *
*/
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define ITERATIONS 30
#define MAX_DATA_TRANS 32*1024*1024
#define MIN_DATA_TRANS 1

int main(int argc, char** argv) {

	int my_rank, world_size, buf_size, dest, tag;
	double start, end;
	void *sendbuf = NULL, *recvbuf = NULL;

	tag = 0;
	buf_size = MIN_DATA_TRANS;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if(world_size < 2) {
		fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
		MPI_ABORT(MPI_COMM_WORLD, 1);
	}

	//Initialize the buffer to send/receive data
	sendbuf = malloc(MAX_DATA_TRANS);
	recvbuf = malloc(MAX_DATA_TRANS*world_size);
	int i = 0, to_rank = 0;
	struct timeval t1, t2;
	if(my_rank == 0)
		printf("Size(bytes)\tLatency(usec)\tBandwidth(MB/s)\n");
	while(buf_size <= MAX_DATA_TRANS) {
		//Start counting time
		if(my_rank == 0) {
			start = MPI_Wtime();
			//gettimeofday(&t1, NULL);
		}

		for(i = 0; i < ITERATIONS; i++) {
			//Send data to all other nodes and also receive data from all other nodes
			MPI_Allgather(sendbuf, buf_size, MPI_BYTE, recvbuf, buf_size*world_size, MPI_BYTE, MPI_COMM_WORLD);
		}

		//End of the communication. Get the time.
		if(my_rank == 0) {
			end = MPI_Wtime();
			double latency = ((end-start)*1000000)/ITERATIONS;
			//gettimeofday(&t2, NULL);
			//double latency = ((t2.tv_sec * 1000000 + t2.tv_usec)  - (t1.tv_sec * 1000000 + t1.tv_usec))/ITERATIONS;
			double bandwidth = ((double)buf_size*1000000)/(latency*1024*1024);
			printf("%d\t\t%f\t\t%f\n", buf_size, latency, bandwidth);
		}

		buf_size = buf_size*2;
	}
	free(recvbuf);
	free(sendbuf);
	MPI_Finalize();
	return 0;
}
