#include "safemalloc.h"

void * safemalloc(int n) 
{
    int rc;
    void * ptr = NULL;

    rc = posix_memalign( &ptr, ALIGNMENT, (size_t) n );

    if ( ptr == NULL )
    {
        fprintf( stderr , "%d bytes could not be allocated \n" , n );
        exit(1);
    }
#ifdef ENOMEM
    else if ( rc==ENOMEM )
    {
        fprintf( stderr , "%d bytes could not be allocated \n" , n );
        exit(1);
    }
#endif
#ifdef EINVAL
    else if ( rc==EINVAL )
    {
        fprintf( stderr , "invalid alignment (%d) requested \n" , ALIGNMENT );
        exit(1);
    }
#endif
    if ( rc!=0 )
    {
        fprintf( stderr , "posix_memalign returned %d \n" , rc );
        exit(1);
    }

    return ptr;
}
