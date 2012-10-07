#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "all.h"

int main (int argc, char* argv[])
{
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

  int iter = 0;
  double norm = 1.0;
  double thresh = 1.0e-7;
  while (norm>thresh)
  {
    /* these are two tasks i am trying to extract parallelism from */
    foo(n,x,y);
    bar(n,x,y);

    for (int i=0; i<n; i++)
      x[i] = y[i];

    norm = 0.0;
    for (int i=0; i<n; i++)
      norm += y[i]*y[i];

    norm = sqrt(norm);

    iter++;
    printf("after %d iterations, norm = %lf \n", iter, norm);
  }

  return 0;
}

