#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	printf("Hello from rank %ld of %ld \n", PAMID_World_rank(), PAMID_World_size() );

	PAMID_Finalize();

	if (PAMID_World_rank()==0) printf("TEST DONE \n");
    fflush(stdout);

	return 0;
}
