#include "safemalloc.h"

#define NO_MEMALIGN

void * safemalloc(size_t n) 
{
    int rc = 0;
    void * ptr = NULL;

    if (n<=0)
    {
        fprintf( stderr , "attempting to allocate %ld bytes \n" , (long)n );
        fflush(stderr);
    }

#ifdef NO_MEMALIGN
    ptr = malloc( n );
#else
    rc = posix_memalign( &ptr , ALIGNMENT , n );
#endif

    if ( ptr == NULL || rc!=0)
    {
        fprintf( stderr , "%ld bytes could not be allocated \n" , (long)n );
        fflush(stderr);
        sleep(1);
        exit(13);
    }

    return ptr;
}
