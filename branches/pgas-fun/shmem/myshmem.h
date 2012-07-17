#include <stdio.h>
#include <stdlib.h>
#include <mpp/shmem.h>

#ifdef OPENSHMEM
void shmem_init(void)
{
    start_pes(0);
    return;
}

void shmem_finalize(void)
{
    return;
}

int num_pes(void)
{
    return _num_pes();
}

int my_pe(void)
{
    return _my_pe();
}
#endif
