#include "torus.h"

void BGQTopo_Init(void)
{
    uint32_t rc;

    Personality_t pers;
    BG_JobCoords_t jobcoords;

    rc = Kernel_GetPersonality(&pers, sizeof(pers));
    assert(rc==0);

    rc = Kernel_JobCoords(&jobcoords);
    assert(rc==0);

    printf("%d: pers.Network_Config.Acoord = %d \n", rank, pers.Network_Config.Acoord);
    printf("%d: pers.Network_Config.Bcoord = %d \n", rank, pers.Network_Config.Bcoord);
    printf("%d: pers.Network_Config.Ccoord = %d \n", rank, pers.Network_Config.Ccoord);
    printf("%d: pers.Network_Config.Dcoord = %d \n", rank, pers.Network_Config.Dcoord);
    printf("%d: pers.Network_Config.Ecoord = %d \n", rank, pers.Network_Config.Ecoord);
    fflush(stdout);

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

        printf("%d: jobcoords.isSubBlock = %d \n", rank, jobcoords.isSubBlock);

        if (jobcoords.isSubBlock==1)
        {
            printf("%d: jobcoords.corner.a = %d \n", rank, jobcoords.corner.a);
            printf("%d: jobcoords.corner.b = %d \n", rank, jobcoords.corner.b);
            printf("%d: jobcoords.corner.c = %d \n", rank, jobcoords.corner.c);
            printf("%d: jobcoords.corner.d = %d \n", rank, jobcoords.corner.d);
            printf("%d: jobcoords.corner.e = %d \n", rank, jobcoords.corner.e);
            printf("%d: jobcoords.corner.t = %d \n", rank, jobcoords.corner.core);
 
            printf("%d: jobcoords.shape.a = %d \n", rank, jobcoords.shape.a);
            printf("%d: jobcoords.shape.b = %d \n", rank, jobcoords.shape.b);
            printf("%d: jobcoords.shape.c = %d \n", rank, jobcoords.shape.c);
            printf("%d: jobcoords.shape.d = %d \n", rank, jobcoords.shape.d);
            printf("%d: jobcoords.shape.e = %d \n", rank, jobcoords.shape.e);
            printf("%d: jobcoords.shape.t = %d \n", rank, jobcoords.shape.core);

            printf("%d: subjob is torus along a = %d \n", rank, ND_GET_TORUS(0,pers.Network_Config.NetFlags) && jobcoords.shape.a==pers.Network_Config.Anodes);
            printf("%d: subjob is torus along b = %d \n", rank, ND_GET_TORUS(1,pers.Network_Config.NetFlags) && jobcoords.shape.b==pers.Network_Config.Bnodes);
            printf("%d: subjob is torus along c = %d \n", rank, ND_GET_TORUS(2,pers.Network_Config.NetFlags) && jobcoords.shape.c==pers.Network_Config.Cnodes);
            printf("%d: subjob is torus along d = %d \n", rank, ND_GET_TORUS(3,pers.Network_Config.NetFlags) && jobcoords.shape.d==pers.Network_Config.Dnodes);
            printf("%d: subjob is torus along e = %d \n", rank, ND_GET_TORUS(4,pers.Network_Config.NetFlags) && jobcoords.shape.e==pers.Network_Config.Enodes);
        }
        fflush(stdout);
    }

    MPI_Finalize();

    return 0;
}

/*=======================================*/
/* routine to return the BGQ core number */
/*=======================================*/
int get_bgq_core(void)
{
#ifdef BGQ
    //int core = Kernel_PhysicalProcessorID();
    int core = Kernel_ProcessorCoreID();
    return core;
#else
    return -1;
#endif
}

/*==========================================*/
/* routine to return the BGQ hwthread (0-3) */
/*==========================================*/
int get_bgq_hwthread(void)
{
#ifdef BGQ
    //int hwthread = Kernel_PhysicalHWThreadID();
    int hwthread = Kernel_ProcessorThreadID();
    return hwthread;
#else
    return -1;
#endif
}

/*======================================================*/
/* routine to return the BGQ virtual core number (0-67) */
/*======================================================*/
int get_bgq_vcore(void)
{
#ifdef BGQ
    int hwthread = Kernel_ProcessorID();
    return hwthread;
#else
    return -1;
#endif
}

