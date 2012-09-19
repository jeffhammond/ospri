#include "pamid.h"

int main(int argc, void * argv[])
{
	PAMID_Initialize();

	printf("Hello from rank %ld of %ld \n", PAMID_World_rank(), PAMID_World_size() );

	PAMID_Finalize();

	return 0;
}
