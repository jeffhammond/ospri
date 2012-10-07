#include <stdio.h>
#include <stdlib.h>

void init(int n, double * x)
{
  for (int i=0; i<n; i++)
    x[i] = rand()/(double)RAND_MAX;

  return;
}
