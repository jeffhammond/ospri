#include <stdio.h>

void foo(int n, double * x, double * y)
{
  for (int i=0; i<n; i++)
    y[i] = x[i]*x[i];

#ifdef DEBUG
  for (int i=0; i<n; i++)
    printf("foo: i = %d x[i] = %lf, y[i] = %lf \n", i, x[i], y[i] );
#endif

  return;
}
