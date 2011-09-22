#include <mpi.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
 int numprocs, myid, granted;

 MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&granted);
 if( granted == MPI_THREAD_SINGLE ) printf("MPI_THREAD_SINGLE\n");
 else if( granted == MPI_THREAD_FUNNELED) printf("MPI_THREAD_FUNNELED\n");
 else if( granted == MPI_THREAD_SERIALIZED) printf("MPI_THREAD_SERIALIZED\n");
 else if( granted == MPI_THREAD_MULTIPLE ) printf("MPI_THREAD_MULTIPLE\n");
 else assert(0);

 //MPI_Init(&argc,&argv);

 MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
 MPI_Comm_rank(MPI_COMM_WORLD,&myid);

 printf("Hello from %d of %d processors\n", myid, numprocs);

 MPI_Finalize();
 return 0;
}
