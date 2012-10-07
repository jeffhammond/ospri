#include <stdio.h>
#include <stdlib.h>

void update(const int n, double * restrict x, double * restrict y)
{
#ifdef _OPENMP
  #pragma omp parallel for private(i) schedule(static,chunk)
#endif
  for (int i=0; i<n; i++)
    y[i] = x[i];

  return;
}
