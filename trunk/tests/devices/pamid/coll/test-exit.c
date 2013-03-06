#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/time.h>

#include <spi/include/kernel/location.h>

int main(int argc, char* argv[])
{
    int rank  = Kernel_GetRank();
    int rc    = ((argc>1) ? atoi(argv[1]) : 0);
    int delay = ((argc>2) ? atoi(argv[2]) : 1800);

    if (rank==0)
    {
        printf("rank %d is exiting now with code %d ...\n", rank, rc);
        fflush(stdout);
        exit(rc);
    }
    else
    {
        printf("rank %d is sleeping for %d seconds \n", rank, delay);
        sleep(delay);
        printf("rank %d is exiting now with code %d ...\n", rank, rc);
        fflush(stdout);
        exit(rc);
    }

    return 0;
}
