#include "safemalloc.h"

void * safemalloc(int n) 
{
    void * ptr = malloc( n );

    if ( ptr == NULL )
    {
        fprintf( stderr , "%d bytes could not be allocated \n" , n );
        exit(1);
    }

    return ptr;
}
