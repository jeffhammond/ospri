#include "alcf-vector.h"

#ifndef REALTYPE
#error Need to define a floating-point type.
#endif

#if REALTYPE == double
#define REALNAME "doubles"
#endif

void vec_scale_c_basic(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a);
void vec_scale_c_unroll2(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a);
void vec_scale_c_unroll4(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a);
void vec_scale_c_unroll8(const unsigned long n, const REALTYPE * RESTRICT x, REALTYPE * RESTRICT y, const REALTYPE a);

int main(int argc, char* argv[])
{
    unsigned long long t0, t1;

    unsigned long n = (argc>1) ? atol(argv[1]) : 1000000;

    const unsigned int warmup = 10;
    unsigned int repetitions = (argc>2) ? atol(argv[2]) : 100;

    double scaling = 7.33;

    printf("vec_scale test for %lu elements \n", n);

    REALTYPE * x = safemalloc(n*sizeof(REALTYPE));
    REALTYPE * y = safemalloc(n*sizeof(REALTYPE));

    for (unsigned int i=0;i<n;i++)
        x[i] = (double) i;

    for (unsigned int i=0;i<n;i++)
        y[i] = (double) -i;

    /* basic */

    for (unsigned int r=0;r<warmup;r++) 
        vec_scale_c_basic(n,x,y,scaling);

    t0 = getticks();
    for (unsigned int r=0;r<repetitions;r++) 
        vec_scale_c_basic(n,x,y,scaling);
    t1 = getticks();
    printf("%20s of %lu %s took %12llu cycles (%6.2lf cycles/element) \n", "vec_scale_c_basic", n, REALNAME , (t1-t0)/repetitions, ((double)t1-t0)/(repetitions*n) );

    /* unroll2 */

    for (unsigned int r=0;r<warmup;r++) 
        vec_scale_c_unroll2(n,x,y,scaling);

    t0 = getticks();
    for (unsigned int r=0;r<repetitions;r++) 
        vec_scale_c_unroll2(n,x,y,scaling);
    t1 = getticks();
    printf("%20s of %lu %s took %12llu cycles (%6.2lf cycles/element) \n", "vec_scale_c_unroll2", n, REALNAME , (t1-t0)/repetitions, ((double)t1-t0)/(repetitions*n) );

    /* unroll4 */

    for (unsigned int r=0;r<warmup;r++) 
        vec_scale_c_unroll4(n,x,y,scaling);

    t0 = getticks();
    for (unsigned int r=0;r<repetitions;r++) 
        vec_scale_c_unroll4(n,x,y,scaling);
    t1 = getticks();
    printf("%20s of %lu %s took %12llu cycles (%6.2lf cycles/element) \n", "vec_scale_c_unroll4", n, REALNAME , (t1-t0)/repetitions, ((double)t1-t0)/(repetitions*n) );

    /* unroll4 */

    for (unsigned int r=0;r<warmup;r++) 
        vec_scale_c_unroll8(n,x,y,scaling);

    t0 = getticks();
    for (unsigned int r=0;r<repetitions;r++) 
        vec_scale_c_unroll8(n,x,y,scaling);
    t1 = getticks();
    printf("%20s of %lu %s took %12llu cycles (%6.2lf cycles/element) \n", "vec_scale_c_unroll8", n, REALNAME , (t1-t0)/repetitions, ((double)t1-t0)/(repetitions*n) );

    /**********/

    free(y);
    free(x);

    return 0;
}
