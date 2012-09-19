#include "pamid.h"

int main(int argc, char * argv[])
{
	PAMID_Initialize();

	int ws = (int)PAMID_World_size();

	const int n = 1000;
	int * in  = malloc(n*sizeof(int));
	int * out = malloc(ws*n*sizeof(int));

	int r = (int)PAMID_World_rank();

	for (int i=0; i<n; i++)
		in[i] = 37373737;

	for (int i=0; i<(ws*n); i++)
		out[i] = -1;

	PAMID_Allgather_world(n*sizeof(int), (void*) in, (void*) out);

	int errors = 0;
	for (int i=0; i<(ws*n); i++)
		if (out[i] != 37373737)
			errors++;

	if (errors>0)
		printf("rank %ld has %d errors \n", PAMID_World_rank(), errors );

	free(out);
	free(in);

	PAMID_Finalize();

	printf("TEST DONE \n");

	return 0;
}
