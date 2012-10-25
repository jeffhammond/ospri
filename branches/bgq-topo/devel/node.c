#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <spi/include/kernel/process.h>
#include <spi/include/kernel/location.h>
#include <firmware/include/personality.h>

int main(int argc, char* argv[])
{
    if (rank==0) printf("%d: Kernel_GetJobID() = %lu \n", rank, Kernel_GetJobID() );

    printf("%d: Kernel_ProcessCount() = %d \n", rank, Kernel_ProcessCount() );
    printf("%d: Kernel_ProcessorCount() = %d \n", rank, Kernel_ProcessorCount() );
    printf("%d: Kernel_ProcessorID() = %d \n", rank, Kernel_ProcessorID() );
    printf("%d: Kernel_ProcessorCoreID() = %d \n", rank, Kernel_ProcessorCoreID() );
    printf("%d: Kernel_ProcessorThreadID() = %d \n", rank, Kernel_ProcessorThreadID() );
    printf("%d: Kernel_BlockThreadId() = %d \n", rank, Kernel_BlockThreadId() );
    printf("%d: Kernel_MyTcoord() = %d \n", rank, Kernel_MyTcoord() );
    printf("%d: Kernel_GetRank() = %d \n", rank, Kernel_GetRank() );

    fflush(stdout);

    return 0;
}
