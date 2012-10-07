#include <stdio.h>

void bar(const int n, double * restrict x, double * restrict y)
{
#ifdef _OPENMP
  #pragma omp parallel for private(i) schedule(static,chunk)
#endif
  for (int i=0; i<n; i++)
    y[i] = x[i]*x[i];

#ifdef DEBUG
  for (int i=0; i<n; i++)
    printf("foo: i = %d x[i] = %lf, y[i] = %lf \n", i, x[i], y[i] );
#endif

  return;
}
