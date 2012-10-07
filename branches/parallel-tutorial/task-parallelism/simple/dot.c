#include <stdio.h>
#include <stdlib.h>

double dot(const int n, double * restrict x)
{
  double r = 0.0;
#ifdef _OPENMP
  #pragma omp parallel for schedule(static,chunk) private(i) reduction(+:r) 
#endif
  for (int i=0; i<n; i++)
    r += x[i]*x[i];

  return r;
}
