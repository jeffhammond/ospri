#include "safemalloc.h"

void * safemalloc(size_t n) 
{
    void * ptr;
    ptr = malloc( n );

    assert( ptr != NULL );

    /* in case assertions are disabled */
    if ( ptr != NULL ) exit(911);

    return ( ptr != NULL ? ptr : NULL);
}
