#include "alcf-vector.h"

#ifndef REALTYPE
#error Need to define a floating-point type.
#endif

#ifndef RESTRICT
#warning Lacking useful anti-aliasing semantics.
#endif

void vec_scale_c_basic(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a)
{
    unsigned long i;

    for (i=0;i<n;i++)
        y[i] = a*x[i];

    return;
}

void vec_scale_c_unroll2(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a)
{
    unsigned long i, r=(n%2), m=(n-r);

    for (i=0;i<m;i+=2)
    {
        y[i]   = a*x[i];
        y[i+1] = a*x[i+1];
    }

    if (r)
        y[n] = a*x[n];

    return;
}

void vec_scale_c_unroll4(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a)
{
    unsigned long i, r=(n%4), m=(n-r);

    for (i=0;i<m;i+=4)
    {
        y[i]   = a*x[i];
        y[i+1] = a*x[i+1];
        y[i+2] = a*x[i+2];
        y[i+3] = a*x[i+3];
    }

    for (i=m;i<n;i++)
        y[i] = a*x[i];

    return;
}

void vec_scale_c_unroll8(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a)
{
    unsigned long i, r=(n%8), m=(n-r);

    for (i=0;i<m;i+=8)
    {
        y[i]   = a*x[i];
        y[i+1] = a*x[i+1];
        y[i+2] = a*x[i+2];
        y[i+3] = a*x[i+3];
        y[i+4] = a*x[i+4];
        y[i+5] = a*x[i+5];
        y[i+6] = a*x[i+6];
        y[i+7] = a*x[i+7];
    }

    for (i=m;i<n;i++)
        y[i] = a*x[i];

    return;
}
