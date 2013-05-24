#include "pamid.h"

int main(int argc, char * argv[])
{
    PAMID_Initialize();

    int rank = (int)PAMID_World_rank();
    int size = (int)PAMID_World_size();

    const int n = 1000;
    int * in  = malloc(n*sizeof(int));
    int * out = malloc(size*n*sizeof(int));
    if (in==NULL || out==NULL) abort();

    for (int i=0; i<n; i++)
        in[i] = 37373737;

    for (int i=0; i<(size*n); i++)
        out[i] = -1;

    PAMID_Allgather_world(n*sizeof(int), (void*) in, (void*) out);

    int errors = 0;
    for (int i=0; i<(size*n); i++)
        if (out[i] != 37373737)
            errors++;

    if (errors>0)
        printf("rank %ld has %d errors \n", PAMID_World_rank(), errors );

    free(out);
    free(in);

    PAMID_Finalize();

    if (rank==0) printf("TEST DONE \n");
    fflush(stdout);

    return 0;
}
