#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	const int n = 1000;
	int * a = malloc(n*sizeof(int));
	int * b = malloc(n*sizeof(int));

	//int r = (int)PAMID_World_rank();
	int s = (int)PAMID_World_size();

	for (int i=0; i<n; i++)
		a[i] = 1;

	for (int i=0; i<n; i++)
		b[i] = -1;

	PAMID_Allreduce_world(n, a, b, PAMI_TYPE_SIGNED_INT, PAMI_DATA_SUM);

	int errors = 0;
	for (int i=0; i<n; i++)
		if (b[i] != s)
			errors++;

	if (errors>0)
		printf("rank %ld has %d errors \n", PAMID_World_rank(), errors );

	free(b);
	free(a);

	PAMID_Finalize();

	printf("TEST DONE \n");

	return 0;
}
