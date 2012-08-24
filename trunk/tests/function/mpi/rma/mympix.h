#include <mpi.h>

#if MPI_VERSION >= 3

#define MPIX_Win_allocate MPI_Win_allocate

#elif defined (MPICH2) && (MPICH2_NUM_VERSION >= MPICH2_CALC_VERSION(1,6,0,0,0))

#include <mpix.h>

#else

static int MPIX_Win_allocate(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win)
{
	int rc = MPI_SUCCESS;

	rc = MPI_Alloc_mem(size, MPI_INFO_NULL, baseptr);
	if (rc!=MPI_SUCCESS)
		return rc;

	rc = MPI_Win_create(baseptr, size, disp_unit, MPI_INFO_NULL, comm, win);
	if (rc!=MPI_SUCCESS)
		return rc;

	return MPI_SUCCESS;
}

#endif

#if UNUSED
int MPE_Put(void * source_buffer,
            int source_count, MPI_Datatype source_type,
            int remote_target, MPI_Aint remote_offset,
            int remote_count, MPI_Datatype remote_type,
            MPI_Win window)
{
	int rc = MPI_SUCCESS;

	rc = MPI_Win_lock(MPI_LOCK_SHARED, remote_target, MPI_MODE_NOCHECK, window);
	if (rc!=MPI_SUCCESS)
		return rc;

	rc = MPI_Put(source_buffer, source_count, source_type, remote_target, remote_offset, remote_count, remote_type, window);

	if (rc!=MPI_SUCCESS)
		return rc;

	rc = MPI_Win_unlock(remote_target, window);
	if (rc!=MPI_SUCCESS)
		return rc;

	return MPI_SUCCESS;
}
#endif
