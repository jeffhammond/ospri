#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	const int n = 1000;
	int * a = malloc(n*sizeof(int));

	int r = (int)PAMID_World_rank();

	if (r==0)
		for (int i=0; i<n; i++)
			a[i] = 37373737;
	else
		for (int i=0; i<n; i++)
			a[i] = -1;

	PAMID_Broadcast_world(0, n*sizeof(int), (void*) a);

	int errors = 0;
	for (int i=0; i<n; i++)
		if (a[i] != 37373737)
			errors++;

	if (errors>0)
		printf("rank %ld has %d errors \n", PAMID_World_rank(), errors );

	free(a);

	PAMID_Finalize();

	printf("TEST DONE \n");

	return 0;
}