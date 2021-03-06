#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "all.h"

int main (int argc, char** argv)
{
  int n = (argc>1 ? atoi(argv[1]) : 1000000);
  printf("n = %d \n", n);

  double * restrict x = malloc(n*sizeof(double));
  double * restrict y = malloc(n*sizeof(double));
  if (x==NULL || y==NULL)
  {
    printf("cannot allocate %ld bytes \n", (long) n*sizeof(double) );
    exit(1);
  }

  init(n,x);

  int iter = 0;
  double thresh = 1.0e-7;
  double norm = 1000*thresh;
  while (norm>thresh)
  {
    /* these are two tasks i am trying to extract parallelism from */
    foo(n,x,y);
    bar(n,x,y);

    /* normally this would have a preconditioner in it... */
    update(n,x,y);

    norm = dot(n,y);
    norm = sqrt(norm);

    iter++;
    printf("after %d iterations, norm = %lf \n", iter, norm);
  }

  return 0;
}

