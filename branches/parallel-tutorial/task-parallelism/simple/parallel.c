#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "all.h"

int main (int argc, char* argv[])
{
#ifdef PARALLEL
  int provided = -1;
  /* FUNNELED is the minimum thread support required if OpenMP is used foo and bar */
  MPI_Init_thread(&argc, &argc, MPI_THREAD_FUNNELED, &provided);
#endif

  int n = (argc>1 ? atoi(argv[1]) : 1000000);
  printf("n = %d \n", n);

  double * x = malloc(n*sizeof(double));
  double * y = malloc(n*sizeof(double));
  if (x==NULL || y==NULL)
  {
    printf("cannot allocate %ld bytes \n", (long) n*sizeof(double) );
    exit(1);
  }

  init(n,x);

#ifdef PARALLEL
  int rank = -1;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size>2)
    printf("this program only parallelizes over 2 ranks (not %d) \n", size);
#endif

  int iter = 0;
  double norm = 1.0;
  double thresh = 1.0e-7;
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

    for (int i=0; i<n; i++)
      x[i] = y[i];

    norm = 0.0;
    for (int i=0; i<n; i++)
      norm += y[i]*y[i];

    norm = sqrt(norm);
#ifdef PARALLEL
    MPI_Allreduce(MPI_IN_PLACE, &norm, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 
#endif
    iter++;
    printf("after %d iterations, norm = %lf \n", iter, norm);
  }

#ifdef PARALLEL
  MPI_Finalize(MPI_COMM_WORLD);
#endif
  return 0;
}
