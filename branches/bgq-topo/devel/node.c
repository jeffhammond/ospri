#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <spi/include/kernel/process.h>
#include <spi/include/kernel/location.h>
#include <firmware/include/personality.h>

int main(int argc, char* argv[])
{
    printf("Kernel_GetJobID() = %lu \n", Kernel_GetJobID() );
    printf("Kernel_ProcessCount() = %d \n", Kernel_ProcessCount() );
    printf("Kernel_ProcessorCount() = %d \n", Kernel_ProcessorCount() );
    printf("Kernel_ProcessorID() = %d \n", Kernel_ProcessorID() );
    printf("Kernel_ProcessorCoreID() = %d \n", Kernel_ProcessorCoreID() );
    printf("Kernel_ProcessorThreadID() = %d \n", Kernel_ProcessorThreadID() );
    printf("Kernel_BlockThreadId() = %d \n", Kernel_BlockThreadId() );
    printf("Kernel_MyTcoord() = %d \n", Kernel_MyTcoord() );
    printf("Kernel_GetRank() = %d \n", Kernel_GetRank() );

    fflush(stdout);

    return 0;
}
