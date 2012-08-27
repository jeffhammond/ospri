#include "safemalloc.h"

void * safemalloc(int n) 
{
    //void * ptr = malloc( n );
    int rc;
    void * ptr;
    rc = posix_memalign( &ptr , ALIGNMENT , n );

    if ( ptr == NULL || n<0 )
    {
        fprintf( stderr , "%d bytes could not be allocated \n" , n );
        exit(n);
    }

    return ptr;
}
