#include <stdio.h>
#include <unistd.h>
#include <spi/include/kernel/location.h>

int main(int argc, char* argv[])
{
    int rank = Kernel_GetRank();

    if (rank==0)
    {
        printf("rank %d is exiting now...\n", rank);
        fflush(stdout);
        exit(0);
    }
    else
    {
        sleep(1800);
        printf("rank %d is exiting now...\n", rank);
        fflush(stdout);
        exit(0);
    }

    return 0;
}
