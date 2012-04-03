#include <stdio.h>
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
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    uint32_t rc;
    Personality_t pers;
    rc = Kernel_GetPersonality(&pers, sizeof(pers));

    if (rc == 0)
    {
        printf("%d: pers.Network_Config.Acoord = %d \n", rank, pers.Network_Config.Acoord);
        printf("%d: pers.Network_Config.Bcoord = %d \n", rank, pers.Network_Config.Bcoord);
        printf("%d: pers.Network_Config.Ccoord = %d \n", rank, pers.Network_Config.Ccoord);
        printf("%d: pers.Network_Config.Dcoord = %d \n", rank, pers.Network_Config.Dcoord);
        printf("%d: pers.Network_Config.Ecoord = %d \n", rank, pers.Network_Config.Ecoord);

        printf("%d: pers.Network_Config.Anodes = %d \n", rank, pers.Network_Config.Anodes);
        printf("%d: pers.Network_Config.Bnodes = %d \n", rank, pers.Network_Config.Bnodes);
        printf("%d: pers.Network_Config.Cnodes = %d \n", rank, pers.Network_Config.Cnodes);
        printf("%d: pers.Network_Config.Dnodes = %d \n", rank, pers.Network_Config.Dnodes);
        printf("%d: pers.Network_Config.Enodes = %d \n", rank, pers.Network_Config.Enodes);

        printf("%d: pers.Network_Config.NetFlags = %llu \n", rank, pers.Network_Config.NetFlags);
        if (rank==0)
        {
            printf("%d: ND_ENABLE_TORUS_DIM_A = %d \n", rank, ND_ENABLE_TORUS_DIM_A);
            printf("%d: ND_ENABLE_TORUS_DIM_B = %d \n", rank, ND_ENABLE_TORUS_DIM_B);
            printf("%d: ND_ENABLE_TORUS_DIM_C = %d \n", rank, ND_ENABLE_TORUS_DIM_C);
            printf("%d: ND_ENABLE_TORUS_DIM_D = %d \n", rank, ND_ENABLE_TORUS_DIM_D);
            printf("%d: ND_ENABLE_TORUS_DIM_E = %d \n", rank, ND_ENABLE_TORUS_DIM_E);

            char t = 1;
            int i;
            for (i=0;i<64;i++) printf("pers.Network_Config.NetFlags >> %d = %d \n", i, (int) t & (pers.Network_Config.NetFlags >> i) );
	}

        printf("%d: pers.Network_Config.Atorus = %d \n", rank, ND_ENABLE_TORUS_DIM_A & pers.Network_Config.NetFlags);
        printf("%d: pers.Network_Config.Btorus = %d \n", rank, ND_ENABLE_TORUS_DIM_B & pers.Network_Config.NetFlags);
        printf("%d: pers.Network_Config.Ctorus = %d \n", rank, ND_ENABLE_TORUS_DIM_C & pers.Network_Config.NetFlags);
        printf("%d: pers.Network_Config.Dtorus = %d \n", rank, ND_ENABLE_TORUS_DIM_D & pers.Network_Config.NetFlags);
        printf("%d: pers.Network_Config.Etorus = %d \n", rank, ND_ENABLE_TORUS_DIM_E & pers.Network_Config.NetFlags);
    }

    fflush(stdout);

    MPI_Finalize();

    return 0;
}
