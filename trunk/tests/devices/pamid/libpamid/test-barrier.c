#include "pamid.h"

int main(int argc, char * argv[])
{
    PAMID_Initialize();

    size_t rank = PAMID_World_rank();

    PAMID_Barrier_world();

    PAMID_Finalize();

    if (rank==0) printf("TEST DONE \n");
    fflush(stdout);

    return 0;
}
