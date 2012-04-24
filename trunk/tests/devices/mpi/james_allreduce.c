#include <stdio.h>
#include <mpi.h>

#define MAXLEN (1024*1024)
#define REPEAT 100

int
main(int argc, char *argv[])
{
 double x[REPEAT][MAXLEN], r[REPEAT][MAXLEN];
 double secs;
 int rank;
 int i, j, len;

 MPI_Init(&argc, &argv);

 MPI_Comm_rank(MPI_COMM_WORLD, &rank);

 for(i=0; i<REPEAT; i++)
   for(j=0; j<MAXLEN; j++) 
     x[i][j] = 1.7*rank + j;

 for(len=1; len<=MAXLEN; len*=2) 
 {

   MPI_Barrier(MPI_COMM_WORLD);
   secs = -MPI_Wtime();
   for(i=0; i<REPEAT; i++) 
     MPI_Allreduce((void *)x[i], (void *)r[i], len, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
   secs += MPI_Wtime();

   if(rank==0)
     printf("%i\t%g\n", len, 1e6*secs/REPEAT);
 }

 MPI_Finalize();
 return 0;
}
