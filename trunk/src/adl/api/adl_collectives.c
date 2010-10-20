/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

/* This is here because the build system does not yet have the necessary
 * logic to set these options for each device. */

#define OSP_USES_MPI_COLLECTIVES

#ifdef OSP_USES_MPI_COLLECTIVES
#include "mpi.h"
#endif

#define ABS(datatype, source, target, count)                                \
   do {                                                                     \
     int i;                                                                 \
     datatype *s = (datatype *) source;                                     \
     datatype *t = (datatype *) target;                                     \
     for(i=0; i<count; i++) t[i] = ( s[i] > 0 ? s[i] : -s[i]);              \
   } while(0)                                                               \

int OSP_Barrier_group(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = MPI_Barrier(MPI_COMM_WORLD);
        switch (status)
        {
            case MPI_ERR_COMM:
                OSPU_ERR_POP(1,"MPI_Barrier returned MPI_ERR_COMM.");
                break;
            default:
                status = OSP_SUCCESS;
                goto fn_exit;
                break;
        }
    }
    else
    {
        OSPU_ERR_POP(1,"OSP_Barrier_group not implemented for non-world groups!");
    }

#else

    /* barrier is meaningless with 1 process */
    if (1==OSPD_Process_total(OSP_GROUP_WORLD)) goto fn_exit;

    status = OSPD_Barrier_group(group);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Barrier_group returned an error\n");

#endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSP_NbBarrier_group(OSP_group_t* group, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES

    OSPU_ERR_POP(1,"OSP_NbBarrier_group not implemented for when OSP_USES_MPI_COLLECTIVES is defined.");

#else

    /* barrier is meaningless with 1 process */
    if ( 1==OSPD_Process_total(OSP_GROUP_WORLD) ) goto fn_exit;

    status = OSPD_NbBarrier_group(group, osp_handle);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbBarrier_group returned an error\n");

#endif

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_Sync_group(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES

    status = OSPD_Flush_group(group);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Flush_group returned an error\n");

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = MPI_Barrier(MPI_COMM_WORLD);
        switch (status)
        {
            case MPI_ERR_COMM:
                OSPU_ERR_POP(1,"MPI_Barrier returned MPI_ERR_COMM.");
                break;
            default:
                status = OSP_SUCCESS;
                goto fn_exit;
                break;
        }
    }
    else
    {
        OSPU_ERR_POP(1,"OSP_Sync_group not implemented for non-world groups!");
    }

#else

    /* no collective bypass for 1 proc here because we will use DCMF for some operations
     * which need to be completed by flush */

    status = OSPD_Sync_group(group);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Sync_group returned an error\n");

#endif


  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_NbSync_group(OSP_group_t* group, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES

    OSPU_ERR_POP(1,"OSP_NbSync_group not implemented for when OSP_USES_MPI_COLLECTIVES is defined.");

#else

    /* no collective bypass for 1 proc here because we will use DCMF for some operations
     * which need to be completed by flush */

    status = OSPD_NbSync_group(group, osp_handle);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbSync_group returned an error\n");

#endif

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_Allreduce_group(OSP_group_t* group,
                       int count,
                       OSP_reduce_op_t osp_op,
                       OSP_datatype_t osp_type,
                       void* in,
                       void* out)
{
#ifdef OSP_USES_MPI_COLLECTIVES
    MPI_Datatype mpi_type;
    MPI_Op mpi_oper;
    int bytes;
    void *in_absolute = NULL;
#endif /* OSP_USES_MPI_COLLECTIVES */
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES
    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        switch (osp_type)
        {
            case OSP_DOUBLE:
                mpi_type = MPI_DOUBLE;
                break;
            case OSP_INT32:
                mpi_type = MPI_LONG;
                break;
            case OSP_INT64:
                mpi_type = MPI_LONG_LONG;
                break;
            case OSP_UINT32:
                mpi_type = MPI_UNSIGNED_LONG;
                break;
            case OSP_UINT64:
                mpi_type = MPI_UNSIGNED_LONG_LONG;
                break;
            case OSP_FLOAT:
                mpi_type = MPI_FLOAT;
                break;
            default:
                OSPU_ERR_POP(status!=OSP_SUCCESS, "Unsupported OSP_datatype\n");
                break;
        }

        switch (osp_op)
        {
            case OSP_SUM:
                mpi_oper = MPI_SUM;
                break;
            case OSP_PROD:
                mpi_oper = MPI_PROD;
                break;
            case OSP_MAX:
                mpi_oper = MPI_MAX;
                break;
            case OSP_MIN:
                mpi_oper = MPI_MIN;
                break;
            case OSP_OR:
                mpi_oper = MPI_LOR;
                break;
            case OSP_MAXABS:
                mpi_oper = MPI_MAX;
                break;
            case OSP_MINABS:
                mpi_oper = MPI_MIN;
                break;
            case OSP_SAME:
                OSPU_ERR_POP(1, "OSP_SAME is not supported when OSP_USES_MPI_COLLECTIVES is defined.\n");
                break;
            default:
                OSPU_ERR_POP(status!=OSP_SUCCESS, "Unsupported OSP_op\n");
                break;
        }
 
        if(osp_op == OSP_MAXABS || osp_op == OSP_MINABS)
        switch (osp_type)
        {
            case OSP_DOUBLE:
                bytes = count * sizeof(double);
                in_absolute = malloc(bytes);
                OSPU_ERR_POP(in_absolute == NULL,
                            "malloc returned error in OSP_Allreduce_group \n");
                ABS(double, in, in_absolute, count);
                in = in_absolute;
                break;
            case OSP_INT32:
                bytes = count * sizeof(int32_t);
                in_absolute = malloc(bytes);
                OSPU_ERR_POP(in_absolute == NULL,
                            "malloc returned error in OSP_Allreduce_group \n");
                ABS(int32_t, in, in_absolute, count);
                in = in_absolute;
                break;
            case OSP_INT64:
                bytes = count * sizeof(int64_t);
                in_absolute = malloc(bytes);
                OSPU_ERR_POP(in_absolute == NULL,
                            "malloc returned error in OSP_Allreduce_group \n");
                ABS(int64_t, in, in_absolute, count);
                in = in_absolute;
                break;
            case OSP_UINT32:
                break;
            case OSP_UINT64:
                break;
            case OSP_FLOAT:
                bytes = count * sizeof(float);
                in_absolute = malloc(bytes);
                OSPU_ERR_POP(in_absolute == NULL,
                            "malloc returned error in OSP_Allreduce_group \n");
                ABS(float, in, in_absolute, count);
                in = in_absolute;
                break;
            default:
                status = OSP_ERROR;
                OSPU_ERR_POP(status != OSP_SUCCESS, "Unsupported OSP_datatype \n");
                break;
        }

        if (in==out) in = MPI_IN_PLACE;
        status = MPI_Allreduce(in,out,count,mpi_type,mpi_oper,MPI_COMM_WORLD);
        switch (status)
        {
            case MPI_ERR_BUFFER:
                OSPU_ERR_POP(1,"MPI_Allreduce returned MPI_ERR_BUFFER.");
                break;
            case MPI_ERR_COUNT:
                OSPU_error_printf("count = %d\n",count);
                OSPU_ERR_POP(1,"MPI_Allreduce returned MPI_ERR_COUNT.");
                break;
            case MPI_ERR_TYPE:
                OSPU_ERR_POP(1,"MPI_Allreduce returned MPI_ERR_TYPE.");
                break;
            case MPI_ERR_OP:
                OSPU_ERR_POP(1,"MPI_Allreduce returned MPI_ERR_OP.");
                break;
            case MPI_ERR_COMM:
                OSPU_ERR_POP(1,"MPI_Allreduce returned MPI_ERR_COMM.");
                break;
            default:
                status = OSP_SUCCESS;
                goto fn_exit;
                break;
        }
    }
    else
    {
        OSPU_ERR_POP(1,"OSP_Allreduce_group not implemented for non-world groups!");
    }

#else

    if (count <= 0) goto fn_exit;

    /* bypass any sort of network API or communication altogether */
    if ( 1==OSPD_Process_total(OSP_GROUP_WORLD) )
    {
        switch (osp_type)
        {
            case OSP_DOUBLE:
                memcpy(in,out,count*sizeof(double));
                break;
            case OSP_INT32:
                memcpy(in,out,count*sizeof(int32_t));
                break;
            case OSP_INT64:
                memcpy(in,out,count*sizeof(int64_t));
                break;
            case OSP_UINT32:
                memcpy(in,out,count*sizeof(uint32_t));
                break;
            case OSP_UINT64:
                memcpy(in,out,count*sizeof(uint64_t));
                break;
            case OSP_FLOAT:
                memcpy(in,out,count*sizeof(float));
                break;
            default:
                OSPU_ERR_POP(status!=OSP_SUCCESS, "Unsupported OSP_datatype\n");
                break;
        }
        goto fn_exit;
    }

    status = OSPD_Allreduce_group(group,
                                 count,
                                 osp_op,
                                 osp_type,
                                 in,
                                 out);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Allreduce_group returned an error\n");

#endif

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_NbAllreduce_group(OSP_group_t* group,
                         int count,
                         OSP_reduce_op_t osp_op,
                         OSP_datatype_t osp_type,
                         void* in,
                         void* out,
                         OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES

    OSPU_ERR_POP(1,"OSP_NbAllreduce_group not implemented for when OSP_USES_MPI_COLLECTIVES is defined.");

#else

    if (count <= 0) goto fn_exit;

    /* bypass any sort of network API or communication altogether */
    if ( 1==OSPD_Process_total(OSP_GROUP_WORLD) )
    {
        switch (osp_type)
        {
            case OSP_DOUBLE:
                memcpy(in,out,count*sizeof(double));
                break;
            case OSP_INT32:
                memcpy(in,out,count*sizeof(int32_t));
                break;
            case OSP_INT64:
                memcpy(in,out,count*sizeof(int64_t));
                break;
            case OSP_UINT32:
                memcpy(in,out,count*sizeof(uint32_t));
                break;
            case OSP_UINT64:
                memcpy(in,out,count*sizeof(uint64_t));
                break;
            case OSP_FLOAT:
                memcpy(in,out,count*sizeof(float));
                break;
            default:
                OSPU_ERR_POP(status!=OSP_SUCCESS, "Unsupported OSP_datatype\n");
                break;
        }
        goto fn_exit;
    }

    status = OSPD_NbAllreduce_group(group,
                                   count,
                                   osp_op,
                                   osp_type,
                                   in,
                                   out,
                                   osp_handle);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbAllreduce_group returned an error\n");

#endif

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_Bcast_group(OSP_group_t* group,
                   int root,
                   int count,
                   void* buffer)
{
#ifdef OSP_USES_MPI_COLLECTIVES
    MPI_Datatype mpi_type = MPI_BYTE;
#endif /* OSP_USES_MPI_COLLECTIVES */
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES
    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = MPI_Bcast(buffer,count,mpi_type,root,MPI_COMM_WORLD);
        switch (status)
        {
            case MPI_ERR_BUFFER:
                OSPU_ERR_POP(1,"MPI_Bcast returned MPI_ERR_BUFFER.");
                break;
            case MPI_ERR_COUNT:
                OSPU_error_printf("count = %d\n",count);
                OSPU_ERR_POP(1,"MPI_Bcast returned MPI_ERR_COUNT.");
                break;
            case MPI_ERR_TYPE:
                OSPU_ERR_POP(1,"MPI_Bcast returned MPI_ERR_TYPE.");
                break;
            case MPI_ERR_ROOT:
                OSPU_ERR_POP(1,"MPI_Bcast returned MPI_ERR_ROOT.");
                break;
            case MPI_ERR_COMM:
                OSPU_ERR_POP(1,"MPI_Bcast returned MPI_ERR_COMM.");
                break;
            default:
                status = OSP_SUCCESS;
                goto fn_exit;
                break;
        }
    }
    else
    {
        OSPU_ERR_POP(1,"OSP_Barrier_group not implemented for non-world groups!");
    }

#else

    if (count <= 0) goto fn_exit;

    /* bypass any sort of network API or communication altogether */
    if ( 1==OSPD_Process_total(OSP_GROUP_WORLD) ) goto fn_exit;

    status = OSPD_Bcast_group(group,
                             root,
                             count,
                             buffer);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Bcast_group returned an error\n");

#endif

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_NbBcast_group(OSP_group_t* group,
                     int root,
                     int count,
                     void* buffer,
                     OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

#ifdef OSP_USES_MPI_COLLECTIVES

    OSPU_ERR_POP(1,"OSP_NbBcast_group not implemented for when OSP_USES_MPI_COLLECTIVES is defined.");

#else

    if (count <= 0) goto fn_exit;

    /* bypass any sort of network API or communication altogether */
    if ( 1==OSPD_Process_total(OSP_GROUP_WORLD) ) goto fn_exit;

    status = OSPD_NbBcast_group(group,
                               root,
                               count,
                               buffer,
                               osp_handle);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbBcast_group returned an error\n");

#endif

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
