#include "pamiu.h"

void * PAMIU_Malloc(size_t n)
{
    void * ptr = NULL;
    int rc = posix_memalign( &ptr , ALIGNMENT , n );

    if ( ptr==NULL || rc!=0 || n<0 )
    {
        fprintf( stderr , "PAMIU_Malloc: %d bytes could not be allocated \n" , n );
        exit(n);
    }

    return ptr;
}

void PAMIU_Free(void * ptr)
{
	free(ptr);

	return;
}
