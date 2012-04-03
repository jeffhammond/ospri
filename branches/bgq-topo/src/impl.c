#include "impl.h"

BGQ_Torus_t info;

void BGQTopo_Init(void)
{
    uint32_t rc;

    Personality_t pers;
    BG_JobCoords_t jobcoords;

    rc = Kernel_GetPersonality(&pers, sizeof(pers));
    assert(rc==0);

    rc = Kernel_JobCoords(&jobcoords);
    assert(rc==0);

    info.Coords[0] = pers.Network_Config.Acoord;
    info.Coords[1] = pers.Network_Config.Bcoord;
    info.Coords[2] = pers.Network_Config.Ccoord;
    info.Coords[3] = pers.Network_Config.Dcoord;
    info.Coords[4] = pers.Network_Config.Ecoord;
    info.Coords[5] = Kernel_ProcessorID();

    info.PartitionSize[0] = pers.Network_Config.Anodes;
    info.PartitionSize[1] = pers.Network_Config.Bnodes;
    info.PartitionSize[2] = pers.Network_Config.Cnodes;
    info.PartitionSize[3] = pers.Network_Config.Dnodes;
    info.PartitionSize[4] = pers.Network_Config.Enodes;
    info.PartitionSize[5] = Kernel_ProcessCount(); 

    info.PartitionTorus[0] = ND_GET_TORUS(0,pers.Network_Config.NetFlags);
    info.PartitionTorus[1] = ND_GET_TORUS(1,pers.Network_Config.NetFlags);
    info.PartitionTorus[2] = ND_GET_TORUS(2,pers.Network_Config.NetFlags);
    info.PartitionTorus[3] = ND_GET_TORUS(3,pers.Network_Config.NetFlags);
    info.PartitionTorus[4] = ND_GET_TORUS(4,pers.Network_Config.NetFlags);
    info.PartitionTorus[5] = 1;

    info.JobSize[0] = jobcoords.shape.a;
    info.JobSize[1] = jobcoords.shape.b;
    info.JobSize[2] = jobcoords.shape.c;
    info.JobSize[3] = jobcoords.shape.d;
    info.JobSize[4] = jobcoords.shape.e;
    info.JobSize[5] = jobcoords.shape.core;

    info.JobTorus[0] = ND_GET_TORUS(0,pers.Network_Config.NetFlags) && jobcoords.shape.a==pers.Network_Config.Anodes;
    info.JobTorus[1] = ND_GET_TORUS(1,pers.Network_Config.NetFlags) && jobcoords.shape.b==pers.Network_Config.Bnodes;
    info.JobTorus[2] = ND_GET_TORUS(2,pers.Network_Config.NetFlags) && jobcoords.shape.c==pers.Network_Config.Cnodes;
    info.JobTorus[3] = ND_GET_TORUS(3,pers.Network_Config.NetFlags) && jobcoords.shape.d==pers.Network_Config.Dnodes;
    info.JobTorus[4] = ND_GET_TORUS(4,pers.Network_Config.NetFlags) && jobcoords.shape.e==pers.Network_Config.Enodes;
    info.JobTorus[5] = 1;

    return;
}
