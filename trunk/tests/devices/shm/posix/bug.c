#include <stdio.h>
#ifdef __bgp__
#  include <spi/kernel_interface.h>
#  include <common/bgp_personality.h>
#  include <common/bgp_personality_inlines.h>
#endif

int main(int argc, char* argv[])
{
#ifdef __bgp__
    _BGP_Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));
    if( BGP_Personality_processConfig(&pers) == _BGP_PERS_PROCESSCONFIG_SMP )
        printf("SMP mode \n");
#endif

    return 0;
}
