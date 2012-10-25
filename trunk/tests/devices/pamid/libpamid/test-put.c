#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	size_t rank = PAMID_World_rank();
	size_t size = PAMID_World_size();

	void ** baseptrs = malloc(sizeof(void*)*size);
	if (baseptrs==NULL) abort();

	size_t n = (argc>1 ? atoi(argv[1]) : 1000000);
	void * ptr = malloc(n);
	if (ptr==NULL) abort();

	PAMID_Allgather_world(sizeof(void*), (void*) ptr, (void*) baseptrs);

	if (ptr!=baseptrs[rank])
		abort();

	free(ptr);
	free(baseptrs);

	PAMID_Finalize();

	printf("TEST DONE \n");

	return 0;
}
