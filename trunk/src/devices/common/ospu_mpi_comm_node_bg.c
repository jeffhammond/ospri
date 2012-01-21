#if !defined(__bgp__) && !defined(__bgq__)
#error This code should only be built on BGP and BGQ.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#if defined(__bgp__)
#  include </bgsys/drivers/ppcfloor/arch/include/spi/kernel_interface.h>
#  include </bgsys/drivers/ppcfloor/arch/include/common/bgp_personality.h>
#  include </bgsys/drivers/ppcfloor/arch/include/common/bgp_personality_inlines.h>
#endif

#if defined(__bgq__)
#  include </bgsys/drivers/ppcfloor/firmware/include/personality.h>
#  include </bgsys/drivers/ppcfloor/spi/include/kernel/process.h>
#  include </bgsys/drivers/ppcfloor/spi/include/kernel/location.h>
#endif

int OSPU_Comm_split_node(MPI_Comm oldcomm, MPI_Comm * newcomm)
{
    int rc;

#if defined(__bgp__)

    _BGP_Personality_t personality;

    Kernel_GetPersonality( &personality, sizeof(personality) );

    int mode = personality.Kernel_Config.ProcessConfig;

    /* SMP mode is trivial */
    if ( mode == _BGP_PERS_PROCESSCONFIG_SMP )
    {
        *newcomm = MPI_COMM_SELF;
        return rc = MPI_SUCCESS;
    }
    else
    {
        int xrank = personality.Network_Config.Xcoord;
        int yrank = personality.Network_Config.Ycoord;
        int zrank = personality.Network_Config.Zcoord;

        int xsize = personality.Network_Config.Xnodes;
        int ysize = personality.Network_Config.Ynodes;
        int zsize = personality.Network_Config.Znodes;

        color = xrank * ysize * zsize
              + yrank * zsize
              + zrank;

        rc = MPI_Comm_split(oldcomm, color, 0, newcomm);
        return rc;
    }

#elif defined(__bgq__)

    /* SMP mode is trivial */
    if ( 1 == Kernel_ProcessCount() )
    {
        *newcomm = MPI_COMM_SELF;
        return rc = MPI_SUCCESS;
    }
    else
    {
        Personality_t personality;
        Kernel_GetPersonality( &personality, sizeof(personality) );

        int arank = personality.Network_Config.Acoord;
        int brank = personality.Network_Config.Bcoord;
        int crank = personality.Network_Config.Ccoord;
        int drank = personality.Network_Config.Dcoord;
        int erank = personality.Network_Config.Ecoord;

        int asize = personality.Network_Config.Anodes;
        int bsize = personality.Network_Config.Bnodes;
        int csize = personality.Network_Config.Cnodes;
        int dsize = personality.Network_Config.Dnodes;
        int esize = personality.Network_Config.Enodes;

        color = arank * bsize * csize * dsize * esize
              + brank * csize * dsize * esize
              + crank * dsize * esize
              + drank * esize
              + erank;

        rc = MPI_Comm_split(oldcomm, color, 0, newcomm);
        return rc;
    }

#endif

    return rc = MPI_SUCCESS;
}
