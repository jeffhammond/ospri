#include "pamid.h"
#include "pamiu.h"

void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
	int * active = (int *) clientdata;
	(*active)--;
}

void cb_local_done (void *ctxt, void * clientdata, pami_result_t err)
{
	pamid_request_t * request = (pamid_request_t*) clientdata;
	((*request).local)--;
}

void cb_remote_done (void *ctxt, void * clientdata, pami_result_t err)
{
	pamid_request_t * request = (pamid_request_t*) clientdata;
	((*request).remote)--;
}

void * PAMIU_Malloc(size_t n)
{
	void * ptr = NULL;
	int rc = posix_memalign( &ptr , ALIGNMENT , n );

	if ( ptr==NULL || rc!=0 || n<0 )
	{
		fprintf( stderr , "PAMIU_Malloc: %ld bytes could not be allocated \n" , (long) n );
		exit(51);
	}

	return ptr;
}

void PAMIU_Free(void * ptr)
{
	free(ptr);

	return;
}
