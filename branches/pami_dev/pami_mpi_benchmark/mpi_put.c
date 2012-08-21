#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#define BUFFER_SIZE 32*1024*1024
#define ITERATIONS 30

int main(int argc, char **argv )
{
	int numprocs;
	int myid;

	int msize = BUFFER_SIZE;
	int mnum = ITERATIONS;  
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Win win;

	if (myid == 0) 
		printf("Using One-sided Fence for fish passing\n");

	//We'll use some arrays of doubles to eat up space
	char *send;
	char *recv;
	send = (char *) calloc (msize, sizeof (char));
	recv = (char *) calloc (msize, sizeof (char));

	//Initialize the buffers that we'll be used for getting.
	int soc = sizeof(char);
	if (myid == 0) {
		MPI_Win_create(send,soc*msize,soc,MPI_INFO_NULL,MPI_COMM_WORLD,&win);
	} else {
		MPI_Win_create(recv,soc*msize,soc,MPI_INFO_NULL,MPI_COMM_WORLD,&win);
	}

	MPI_Win_fence(MPI_MODE_NOPRECEDE,win);
	int i, iter;
	if (myid == 0) {
		double start, stop, diff;
		for(msize = 1; msize <= BUFFER_SIZE; msize = msize*2) {
			start = MPI_Wtime();
			for (i=0; i < mnum; i++) {
				MPI_Win_fence(0,win);
				MPI_Put(recv,msize,MPI_BYTE,1,0,msize,MPI_BYTE,win);
				MPI_Win_fence(0, win);
			}
			stop = MPI_Wtime();
			diff = (stop-start)/mnum;
			printf("%d %7.4f %8.8f\n", msize, diff*1000000, msize/(1024*1024*diff));
		}  
	} else {
		for(msize = 1; msize <= BUFFER_SIZE; msize = msize*2) {
			for (i=0; i < mnum; i++) {
				MPI_Win_fence(0,win);
				//MPI_Put(send,msize,MPI_BYTE,0,0,msize,MPI_BYTE,win);
				MPI_Win_fence(0,win);
    			}
		}
	}
	//MPI_Win_fence(0, win);
	MPI_Win_free(&win);

	MPI_Finalize();
	return 0;
}
