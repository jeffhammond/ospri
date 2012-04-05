#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include <process.h>
#include <location.h>
#include <personality.h>

/**************************************************/

#define F77NAME((lower),(upper)) lower##_

/**************************************************/

typedef struct
{
    int32_t Coords[6];
    int32_t PartitionSize[6];
    int32_t PartitionTorus[6];
    int32_t JobSize[6];
    int32_t JobTorus[6];
}
BGQ_Torus_t;

BGQ_Torus_t info;

void Q5D_Init(void)
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
    info.PartitionTorus[5] = 0;

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
    info.JobTorus[5] = 0;

    return;
}

/* Fortran interface */

void F77NAME(q5d_init,Q5D_INIT)
{
    Q5D_Init();
    return;
}

/* C implementation */

void Q5D_Torus_coords(int32_t coords[])
{
    coords[0] = info.Coords[0];
    coords[1] = info.Coords[1];
    coords[2] = info.Coords[2];
    coords[3] = info.Coords[3];
    coords[4] = info.Coords[4];
    coords[5] = info.Coords[5];
    return;
}

void Q5D_Partition_size(int32_t coords[])
{
    coords[0] = info.PartitionSize[0];
    coords[1] = info.PartitionSize[1];
    coords[2] = info.PartitionSize[2];
    coords[3] = info.PartitionSize[3];
    coords[4] = info.PartitionSize[4];
    coords[5] = info.PartitionSize[5];
    return;
}

void Q5D_Partition_isTorus(int32_t coords[])
{
    coords[0] = info.PartitionTorus[0];
    coords[1] = info.PartitionTorus[1];
    coords[2] = info.PartitionTorus[2];
    coords[3] = info.PartitionTorus[3];
    coords[4] = info.PartitionTorus[4];
    coords[5] = info.PartitionTorus[5];
    return;
}

void Q5D_Job_size(int32_t coords[])
{
    coords[0] = info.JobSize[0];
    coords[1] = info.JobSize[1];
    coords[2] = info.JobSize[2];
    coords[3] = info.JobSize[3];
    coords[4] = info.JobSize[4];
    coords[5] = info.JobSize[5];
    return;
}

void Q5D_Job_isTorus(int32_t coords[])
{
    coords[0] = info.JobTorus[0];
    coords[1] = info.JobTorus[1];
    coords[2] = info.JobTorus[2];
    coords[3] = info.JobTorus[3];
    coords[4] = info.JobTorus[4];
    coords[5] = info.JobTorus[5];
    return;
}

int32_t Q5D_Core_id(void)
{
    /* routine to return the BGQ core number (0-15) */
    return (int32_t) Kernel_ProcessorCoreID();
}

int32_t Q5D_Thread_id(void)
{
    /* routine to return the BGQ virtual core number (0-67) */
    return (int32_t) Kernel_ProcessorID();
}

/* Fortran 64-bit implementation */

void F77NAME(q5d_torus_coords64,Q5D_TORUS_COORDS64) (int64_t * coords)
{
    coords[0] = (int64_t) info.Coords[0];
    coords[1] = (int64_t) info.Coords[1];
    coords[2] = (int64_t) info.Coords[2];
    coords[3] = (int64_t) info.Coords[3];
    coords[4] = (int64_t) info.Coords[4];
    coords[5] = (int64_t) info.Coords[5];
    return;
}

void F77NAME(q5d_partition_size64,Q5D_PARTITION_SIZE64) (int64_t * coords)
{
    coords[0] = (int64_t) info.PartitionSize[0];
    coords[1] = (int64_t) info.PartitionSize[1];
    coords[2] = (int64_t) info.PartitionSize[2];
    coords[3] = (int64_t) info.PartitionSize[3];
    coords[4] = (int64_t) info.PartitionSize[4];
    coords[5] = (int64_t) info.PartitionSize[5];
    return;
}

void F77NAME(q5d_partition_torus64,Q5D_PARTITION_TORUS64) (int64_t * coords)
{
    coords[0] = (int64_t) info.PartitionTorus[0];
    coords[1] = (int64_t) info.PartitionTorus[1];
    coords[2] = (int64_t) info.PartitionTorus[2];
    coords[3] = (int64_t) info.PartitionTorus[3];
    coords[4] = (int64_t) info.PartitionTorus[4];
    coords[5] = (int64_t) info.PartitionTorus[5];
    return;
}

void F77NAME(q5d_job_size64,Q5D_JOB_SIZE64) (int64_t * coords)
{
    coords[0] = (int64_t) info.JobSize[0];
    coords[1] = (int64_t) info.JobSize[1];
    coords[2] = (int64_t) info.JobSize[2];
    coords[3] = (int64_t) info.JobSize[3];
    coords[4] = (int64_t) info.JobSize[4];
    coords[5] = (int64_t) info.JobSize[5];
    return;
}

void F77NAME(q5d_job_torus64,Q5D_JOB_TORUS64) (int64_t * coords)
{
    coords[0] = (int64_t) info.JobTorus[0];
    coords[1] = (int64_t) info.JobTorus[1];
    coords[2] = (int64_t) info.JobTorus[2];
    coords[3] = (int64_t) info.JobTorus[3];
    coords[4] = (int64_t) info.JobTorus[4];
    coords[5] = (int64_t) info.JobTorus[5];
    return;
}

int64_t F77NAME(q5d_core_id64,Q5D_CORE_ID64) (void)
{
    /* routine to return the BGQ core number (0-15) */
    return (int64_t) Kernel_ProcessorCoreID();
}

int64_t F77NAME(q5d_thread_id64,Q5D_THREAD_ID64) (void)
{
    /* routine to return the BGQ virtual core number (0-67) */
    return (int64_t) Kernel_ProcessorID();
}

/* Fortran 32-bit implementation */

void F77NAME(q5d_torus_coords32,Q5D_TORUS_COORDS32) (int32_t * coords)
{
    coords[0] = info.Coords[0];
    coords[1] = info.Coords[1];
    coords[2] = info.Coords[2];
    coords[3] = info.Coords[3];
    coords[4] = info.Coords[4];
    coords[5] = info.Coords[5];
    return;
}

void F77NAME(q5d_partition_size32,Q5D_PARTITION_SIZE32) (int32_t * coords)
{
    coords[0] = info.PartitionSize[0];
    coords[1] = info.PartitionSize[1];
    coords[2] = info.PartitionSize[2];
    coords[3] = info.PartitionSize[3];
    coords[4] = info.PartitionSize[4];
    coords[5] = info.PartitionSize[5];
    return;
}

void F77NAME(q5d_partition_torus32,Q5D_PARTITION_TORUS32) (int32_t * coords)
{
    coords[0] = info.PartitionTorus[0];
    coords[1] = info.PartitionTorus[1];
    coords[2] = info.PartitionTorus[2];
    coords[3] = info.PartitionTorus[3];
    coords[4] = info.PartitionTorus[4];
    coords[5] = info.PartitionTorus[5];
    return;
}

void F77NAME(q5d_job_size32,Q5D_JOB_SIZE32) (int32_t * coords)
{
    coords[0] = info.JobSize[0];
    coords[1] = info.JobSize[1];
    coords[2] = info.JobSize[2];
    coords[3] = info.JobSize[3];
    coords[4] = info.JobSize[4];
    coords[5] = info.JobSize[5];
    return;
}

void F77NAME(q5d_job_torus32,Q5D_JOB_TORUS32) (int32_t * coords)
{
    coords[0] = info.JobTorus[0];
    coords[1] = info.JobTorus[1];
    coords[2] = info.JobTorus[2];
    coords[3] = info.JobTorus[3];
    coords[4] = info.JobTorus[4];
    coords[5] = info.JobTorus[5];
    return;
}

int32_t F77NAME(q5d_core_id32,Q5D_CORE_ID32) (void)
{
    /* routine to return the BGQ core number (0-15) */
    return (int32_t) Kernel_ProcessorCoreID();
}

int32_t F77NAME(q5d_thread_id32,Q5D_THREAD_ID32) (void)
{
    /* routine to return the BGQ virtual core number (0-67) */
    return (int32_t) Kernel_ProcessorID();
}

#if defined(__cplusplus)
}
#endif
