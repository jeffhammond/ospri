#include "safemalloc.h"

void * safemalloc(size_t n)
{
    void * ptr = NULL;

#ifdef USE_MALLOC
    ptr = malloc(n);
#else
    int rc = posix_memalign(&ptr, ALIGNMENT, n);

    if (rc!=0)
    {
        fprintf(stderr, "posix_memalign returned %d \n", rc);
        exit(1);
    }
#endif

    if (ptr==NULL)
    {
        fprintf(stderr, "%ld bytes could not be allocated \n" , (long)n );
        exit(1);
    }

    return ptr;
}
