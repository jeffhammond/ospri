#include "safemalloc.h"

void * safemalloc(size_t n) 
{
#ifdef NO_MEMALIGN
    void * ptr = malloc( n );
#else
    int rc;
    void * ptr;
    rc = posix_memalign( &ptr , ALIGNMENT , n );

    if ( ptr == NULL || n<0 )
    {
        fprintf( stderr , "%ld bytes could not be allocated \n" , (long)n );
        exit(n);
    }
#endif
    return ptr;
}
