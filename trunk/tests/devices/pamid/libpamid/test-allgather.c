#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	PAMID_Barrier_world();

	PAMID_Finalize();

	printf("TEST DONE \n");

	return 0;
}
