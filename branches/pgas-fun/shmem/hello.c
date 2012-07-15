#include <stdio.h>
#include <stdlib.h>
#include <mpp/shmem.h>

int main(int argc, char* argv[])
{
    start_pes(0);

#ifdef OPENSHMEM
    printf("Hello world: I am PE %d of %d.\n", _my_pe(), _num_pes());
#else
    printf("Hello world: I am PE %d of %d.\n", my_pe(), num_pes());
#endif

    shmem_barrier_all();

    return 0;
}
