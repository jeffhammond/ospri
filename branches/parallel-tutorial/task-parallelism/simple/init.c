#include <stdio.h>
#include <stdlib.h>

void init(const int n, double * restrict x)
{
#ifdef _OPENMP
  #pragma omp parallel for private(i) schedule(static,chunk)
#endif
  for (int i=0; i<n; i++)
    x[i] = rand()/(double)RAND_MAX;

  return;
}
