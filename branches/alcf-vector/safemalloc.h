#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int posix_memalign(void **memptr, size_t alignment, size_t size);

#define ALIGNMENT 128

void * safemalloc(int n);
