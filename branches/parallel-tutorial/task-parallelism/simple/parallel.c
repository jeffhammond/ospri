#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include "all.h"

int main (int argc, char** argv)
{
  int rank = 0;
  int size = 1;

#ifdef PARALLEL
  int provided = -1;
  /* FUNNELED is the minimum thread support required if OpenMP is used foo and bar */
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif

  if (rank==0 && size>2)
    printf("this program only parallelizes over 2 ranks (not %d) \n", size);

  int n = (argc>1 ? atoi(argv[1]) : 1000000);
  if (rank==0)
    printf("n = %d \n", n);

  double * restrict x = malloc(n*sizeof(double));
  double * restrict y = malloc(n*sizeof(double));
  if (x==NULL || y==NULL)
  {
    printf("cannot allocate %ld bytes \n", (long) n*sizeof(double) );
    exit(1);
  }

  /* alternatively, one can initialize on rank 0 and broadcast */
  init(n,x);

  int iter = 0;
  double thresh = 1.0e-7;
  double norm = 1000*thresh;
  while (norm>thresh)
  {
    /* these are two tasks i am trying to extract parallelism from */
#ifdef PARALLEL
    if (rank==0)
#endif
      foo(n,x,y);
#ifdef PARALLEL
    else if (rank==1)
#endif
      bar(n,x,y);

    /* normally this would have a preconditioner in it... */
    update(n,x,y);

    norm = dot(n,y);
    norm = sqrt(norm);

    iter++;
#ifdef PARALLEL
    MPI_Allreduce(MPI_IN_PLACE, &norm, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 
    if (rank==0)
#endif
      printf("after %d iterations, norm = %lf \n", iter, norm);
  }

#ifdef PARALLEL
  MPI_Finalize();
#endif
  return 0;
}
