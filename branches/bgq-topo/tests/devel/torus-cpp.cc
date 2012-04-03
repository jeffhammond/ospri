#include <stdio.h>

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
    uint32_t rc;
    Personality_t pers;
    rc = Kernel_GetPersonality(&pers, sizeof(Personality_t));

    bool _blkTorus[5];

    if (rc == 0)
    {
        _blkTorus[0] = (bool) (ND_ENABLE_TORUS_DIM_A & pers.Network_Config.NetFlags);
        _blkTorus[1] = (bool) (ND_ENABLE_TORUS_DIM_B & pers.Network_Config.NetFlags);
        _blkTorus[2] = (bool) (ND_ENABLE_TORUS_DIM_C & pers.Network_Config.NetFlags);
        _blkTorus[3] = (bool) (ND_ENABLE_TORUS_DIM_D & pers.Network_Config.NetFlags);
        _blkTorus[4] = (bool) (ND_ENABLE_TORUS_DIM_E & pers.Network_Config.NetFlags);
        fprintf(stderr, "BGQPersonality() _blkTorus[0] %d, _blkTorus[1] %d, _blkTorus[2] %d, _blkTorus[3] %d, _blkTorus[4] %d\n", 
                                          _blkTorus[0], _blkTorus[1], _blkTorus[2], _blkTorus[3], _blkTorus[4]);
    }

    fflush(stdout);

    return 0;
}
