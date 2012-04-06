/* Fortran implementation */

/**************************************************/

#if (UNDERSCORE==0)
  #define F77NAME(lower,upper,BITS) lower##BITS
#elif (UNDERSCORE==1)
  #define F77NAME(lower,upper,BITS) lower##BITS##_
#elif (UNDERSCORE==2)
  #define F77NAME(lower,upper,BITS) lower##BITS##__
#else
  #define Are you crazy?
#endif

#if (BITS==32)
  #define F77INT int32_t
#elif (BITS=64)
  #define F77INT int64_t
#else
  #error Invalid bits!
#endif

/**************************************************/

void F77NAME(q5d_torus_coords,Q5D_TORUS_COORDS,BITS) (F77INT * coords)
{
    coords[0] = (F77INT) info.Coords[0];
    coords[1] = (F77INT) info.Coords[1];
    coords[2] = (F77INT) info.Coords[2];
    coords[3] = (F77INT) info.Coords[3];
    coords[4] = (F77INT) info.Coords[4];
    coords[5] = (F77INT) info.Coords[5];
    return;
}

void F77NAME(q5d_partition_size,Q5D_PARTITION_SIZE,BITS) (F77INT * coords)
{
    coords[0] = (F77INT) info.PartitionSize[0];
    coords[1] = (F77INT) info.PartitionSize[1];
    coords[2] = (F77INT) info.PartitionSize[2];
    coords[3] = (F77INT) info.PartitionSize[3];
    coords[4] = (F77INT) info.PartitionSize[4];
    coords[5] = (F77INT) info.PartitionSize[5];
    return;
}

void F77NAME(q5d_partition_torus,Q5D_PARTITION_TORUS,BITS) (F77INT * coords)
{
    coords[0] = (F77INT) info.PartitionTorus[0];
    coords[1] = (F77INT) info.PartitionTorus[1];
    coords[2] = (F77INT) info.PartitionTorus[2];
    coords[3] = (F77INT) info.PartitionTorus[3];
    coords[4] = (F77INT) info.PartitionTorus[4];
    coords[5] = (F77INT) info.PartitionTorus[5];
    return;
}

void F77NAME(q5d_job_size,Q5D_JOB_SIZE,BITS) (F77INT * coords)
{
    coords[0] = (F77INT) info.JobSize[0];
    coords[1] = (F77INT) info.JobSize[1];
    coords[2] = (F77INT) info.JobSize[2];
    coords[3] = (F77INT) info.JobSize[3];
    coords[4] = (F77INT) info.JobSize[4];
    coords[5] = (F77INT) info.JobSize[5];
    return;
}

void F77NAME(q5d_job_torus,Q5D_JOB_TORUS,BITS) (F77INT * coords)
{
    coords[0] = (F77INT) info.JobTorus[0];
    coords[1] = (F77INT) info.JobTorus[1];
    coords[2] = (F77INT) info.JobTorus[2];
    coords[3] = (F77INT) info.JobTorus[3];
    coords[4] = (F77INT) info.JobTorus[4];
    coords[5] = (F77INT) info.JobTorus[5];
    return;
}

F77INT F77NAME(q5d_core_id,Q5D_CORE_ID,BITS) (void)
{
    /* routine to return the BGQ core number (0-15) */
    return (F77INT) Kernel_ProcessorCoreID();
}

F77INT F77NAME(q5d_thread_id,Q5D_THREAD_ID,BITS) (void)
{
    /* routine to return the BGQ virtual core number (0-67) */
    return (F77INT) Kernel_ProcessorID();
}
