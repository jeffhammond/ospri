#ifndef ALL_H
#define ALL_H
void init(int n, double * x);
void update(const int n, double * restrict x, double * restrict y);
double dot(const int n, double * restrict x);
void foo(int n, double * x, double * y);
void bar(int n, double * x, double * y);
#endif
