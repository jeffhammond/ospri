#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	size_t rank = PAMID_World_rank();
	size_t size = PAMID_World_size();

	size_t n = (argc>1 ? atoi(argv[1]) : 1000000);
	void * ptr = malloc(n);
	if (ptr==NULL) abort();

	void ** baseptrs = malloc(sizeof(void*)*size);
	if (baseptrs==NULL) abort();

	PAMID_Allgather_world(sizeof(void*), &ptr, baseptrs);

    printf("%ld: ptr = %p baseptrs[%ld] = %p \n", rank, ptr, rank, baseptrs[rank] );

	free(ptr);
	free(baseptrs);

	PAMID_Finalize();

	if (rank==0) printf("TEST DONE \n");

	return 0;
}
