#include "safemalloc.h"

void * safemalloc(size_t n) 
{
    //void * ptr = malloc( n );
    int rc;
    void * ptr;
    rc = posix_memalign( &ptr , ALIGNMENT , n );

    if ( ptr == NULL || n<0 )
    {
        fprintf( stderr , "%ld bytes could not be allocated \n" , (long)n );
        exit(n);
    }

    return ptr;
}
