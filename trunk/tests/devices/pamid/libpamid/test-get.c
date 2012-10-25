#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	PAMID_World_rank();
	PAMID_World_size();

	PAMID_Allgather_world(n*sizeof(int), (void*) in, (void*) out);


	PAMID_Finalize();

	printf("TEST DONE \n");

	return 0;
}
