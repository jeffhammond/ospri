#include "safemalloc.h"

void * safemalloc(size_t n)
{
    //void * ptr = malloc( n );
    int rc;
    void * ptr;
    rc = posix_memalign( &ptr , ALIGNMENT , n );

    if ( ptr == NULL )
    {
        fprintf( stderr , "%ld bytes could not be allocated \n" , (long)n );
        exit(1);
    }

    return ptr;
}
