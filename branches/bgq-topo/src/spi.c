#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <mpi.h>

#ifdef USING_PROPER_INCLUDE_PATHS

/***************************************************************
 * Your Makefile needs something like this:                    *
 * INCLUDE  = -I/bgsys/drivers/ppcfloor                        *
 * INCLUDE += -I/bgsys/drivers/ppcfloor/firmware/include       *
 * INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel     *
 * INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk *
 ***************************************************************/

#include <process.h>
#include <location.h>
#include <personality.h>

#else

#include </bgsys/drivers/ppcfloor/spi/include/kernel/process.h>
#include </bgsys/drivers/ppcfloor/spi/include/kernel/location.h>
#include </bgsys/drivers/ppcfloor/firmware/include/personality.h>

#endif

int main(int argc, char* argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank==0) printf("world size = %d \n", size);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);
    sleep(1);

    uint32_t rc;
    Personality_t pers;
    //rc = Kernel_GetPersonality(&pers, sizeof(pers));
    rc = CNK_SPI_SYSCALL_2(GET_PERSONALITY, (uintptr_t)&pers, (uint64_t)sizeof(pers));

    if (rc == 0)
    {
        printf("%d: pers.Network_Config.Acoord = %d \n", rank, pers.Network_Config.Acoord);
        printf("%d: pers.Network_Config.Bcoord = %d \n", rank, pers.Network_Config.Bcoord);
        printf("%d: pers.Network_Config.Ccoord = %d \n", rank, pers.Network_Config.Ccoord);
        printf("%d: pers.Network_Config.Dcoord = %d \n", rank, pers.Network_Config.Dcoord);
        printf("%d: pers.Network_Config.Ecoord = %d \n", rank, pers.Network_Config.Ecoord);

        fflush(stdout);
        MPI_Barrier(MPI_COMM_WORLD);
        sleep(1);

        if (rank==0)
        {
            printf("%d: pers.Network_Config.Anodes = %d \n", rank, pers.Network_Config.Anodes);
            printf("%d: pers.Network_Config.Bnodes = %d \n", rank, pers.Network_Config.Bnodes);
            printf("%d: pers.Network_Config.Cnodes = %d \n", rank, pers.Network_Config.Cnodes);
            printf("%d: pers.Network_Config.Dnodes = %d \n", rank, pers.Network_Config.Dnodes);
            printf("%d: pers.Network_Config.Enodes = %d \n", rank, pers.Network_Config.Enodes);

            printf("%d: ND_GET_TORUS(0,pers.Network_Config.NetFlags) = %d \n", rank, ND_GET_TORUS(0,pers.Network_Config.NetFlags) );
            printf("%d: ND_GET_TORUS(1,pers.Network_Config.NetFlags) = %d \n", rank, ND_GET_TORUS(1,pers.Network_Config.NetFlags) );
            printf("%d: ND_GET_TORUS(2,pers.Network_Config.NetFlags) = %d \n", rank, ND_GET_TORUS(2,pers.Network_Config.NetFlags) );
            printf("%d: ND_GET_TORUS(3,pers.Network_Config.NetFlags) = %d \n", rank, ND_GET_TORUS(3,pers.Network_Config.NetFlags) );
            printf("%d: ND_GET_TORUS(4,pers.Network_Config.NetFlags) = %d \n", rank, ND_GET_TORUS(4,pers.Network_Config.NetFlags) );
	}
    }

    fflush(stdout);

    MPI_Finalize();

    return 0;
}
