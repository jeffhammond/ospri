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
#define OSPD_IMPLEMENTS_PUTACCS

#if defined OSPD_IMPLEMENTS_PUTACCS

int OSP_PutAccS(int target,
               int stride_level,
               int *block_sizes,
               void* source_ptr,
               int *src_stride_ar,
               void* target_ptr,
               int *trg_stride_ar,
               OSP_datatype_t osp_type,
               void* scaling)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
      int i, bytes = 1;
      for (i = 0; i <= stride_levels; i++) total_bytes *= count[i];
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUTACCS, target, total_bytes);
    }
#   endif

    /*Check if it is a contiguous transfer, issue a contiguous op*/
    if(stride_level == 0)
    {
        /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
        if (target == my_rank && ospu_settings.network_bypass)
        {
           status = OSPU_Acc_local(source_ptr,
                                   target_ptr,
                                   block_sizes[0],
                                   osp_type,
                                   scaling);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Acc_local returned an error\n");
        }
        else
        { 
           status = OSPD_PutAcc(target,
                               source_ptr,
                               target_ptr,
                               block_sizes[0],
                               osp_type,
                               scaling);
           OSPU_ERR_POP((status!=OSP_SUCCESS), "OSPD_PutAcc returned error\n");
        }
        goto fn_exit;
    }

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_AccS_local(stride_level,
                                 block_sizes,
                                 source_ptr,
                                 src_stride_ar,
                                 target_ptr,
                                 trg_stride_ar,
                                 osp_type,
                                 scaling);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_AccS_local returned an error\n");
    }
    else
    {
        status = OSPD_PutAccS(target,
                             stride_level,
                             block_sizes,
                             source_ptr,
                             src_stride_ar,
                             target_ptr,
                             trg_stride_ar,
                             osp_type,
                             scaling);
        OSPU_ERR_POP(status, "OSPD_PutAccS returned error\n");
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_NbPutAccS(int target,
                 int stride_level,
                 int *block_sizes,
                 void* source_ptr,
                 int *src_stride_ar,
                 void* target_ptr,
                 int *trg_stride_ar,
                 OSP_datatype_t osp_type,
                 void* scaling,
                 OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
      int i, bytes = 1;
      for (i = 0; i <= stride_levels; i++) total_bytes *= count[i];
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBPUTACCS, target, total_bytes);
    }
#   endif

    /*Check if it is a contiguous transfer, issue a contiguous op*/
    if(stride_level == 0)
    {
        /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
        if (target == my_rank && ospu_settings.network_bypass)
        {
           status = OSPU_Acc_local(source_ptr,
                                   target_ptr,
                                   block_sizes[0],
                                   osp_type,
                                   scaling);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Acc_local returned an error\n");
        }
        else
        {
           status = OSPD_NbPutAcc(target,
                                 source_ptr,
                                 target_ptr,
                                 block_sizes[0],
                                 osp_type,
                                 scaling,
		   	         osp_handle);
           OSPU_ERR_POP((status!=OSP_SUCCESS), "OSPD_PutAcc returned error\n");
        }
        goto fn_exit;
    }

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_AccS_local(stride_level,
                                 block_sizes,
                                 source_ptr,
                                 src_stride_ar,
                                 target_ptr,
                                 trg_stride_ar,
                                 osp_type,
                                 scaling);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_AccS_local returned an error\n");
    }
    else
    {
        status = OSPD_NbPutAccS(target,
                               stride_level,
                               block_sizes,
                               source_ptr,
                               src_stride_ar,
                               target_ptr,
                               trg_stride_ar,
                               osp_type,
                               scaling,
                               osp_handle);
        OSPU_ERR_POP(status, "NbPutAccS returned error\n");
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#else

int OSPI_Recursive_PutAcc(int target,
                         int stride_level,
                         int *block_sizes,
                         void* source_ptr,
                         int *src_stride_ar,
                         void* target_ptr,
                         int *trg_stride_ar,
                         OSP_datatype_t osp_type,
                         void* scaling,
                         OSP_handle_t osp_handle)
{
    int i, status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    if (stride_level > 0)
    {
        for (i = 0; i < block_sizes[stride_level]; i++)
        {
            status = OSPI_Recursive_PutAcc(target,
                                          stride_level - 1,
                                          block_sizes,
                                          (void *) ((size_t) source_ptr + i * src_stride_ar[stride_level - 1]),
                                          src_stride_ar,
                                          (void *) ((size_t) target_ptr + i * trg_stride_ar[stride_level - 1]),
                                          trg_stride_ar,
                                          osp_type,
                                          scaling,
                                          osp_handle);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                        "OSPI_Recursive_PutAcc returned error in OSPI_Recursive_PutAcc.\n");
        }
    }
    else
    {
        status = OSPD_NbPutAcc(target,
                              source_ptr,
                              target_ptr,
                              block_sizes[0],
                              osp_type,
                              scaling,
                              osp_handle);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbPutAcc returned with an error \n");
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_PutAccS(int target,
               int stride_level,
               int *block_sizes,
               void* source_ptr,
               int *src_stride_ar,
               void* target_ptr,
               int *trg_stride_ar,
               OSP_datatype_t osp_type,
               void* scaling)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
      int i, bytes = 1;
      for (i = 0; i <= stride_levels; i++) total_bytes *= count[i];
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUTACCS, target, total_bytes);
    }
#   endif

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_AccS_local(stride_level,
                                 block_sizes,
                                 source_ptr,
                                 src_stride_ar,
                                 target_ptr,
                                 trg_stride_ar,
                                 osp_type,
                                 scaling);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_AccS_local returned an error\n");
    }
    else
    {
        status = OSPD_Allocate_handle(&osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Allocate_handle returned error\n");

        status = OSPI_Recursive_PutAcc(target,
                                      stride_level,
                                      block_sizes,
                                      source_ptr,
                                      src_stride_ar,
                                      target_ptr,
                                      trg_stride_ar,
                                      osp_type,
                                      scaling,
                                      osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPI_Recursive_PutAcc returned error\n");

        status = OSPD_Wait_handle(osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Wait_handle returned error\n");
    }

    fn_exit:
    /* Could also test for NULL, assuming we set it as such in the declaration. */
    if(target == my_rank && ospu_settings.network_bypass) OSPD_Release_handle(osp_handle);
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_NbPutAccS(int target,
                 int stride_level,
                 int *block_sizes,
                 void* source_ptr,
                 int *src_stride_ar,
                 void* target_ptr,
                 int *trg_stride_ar,
                 OSP_datatype_t osp_type,
                 void* scaling,
                 OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
      int i, bytes = 1;
      for (i = 0; i <= stride_levels; i++) total_bytes *= count[i];
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBPUTACCS, target, total_bytes);
    }
#   endif

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_AccS_local(stride_level,
                                 block_sizes,
                                 source_ptr,
                                 src_stride_ar,
                                 target_ptr,
                                 trg_stride_ar,
                                 osp_type,
                                 scaling);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_AccS_local returned an error\n");
    }
    else
    {
        status = OSPI_Recursive_PutAcc(target,
                                      stride_level,
                                      block_sizes,
                                      source_ptr,
                                      src_stride_ar,
                                      target_ptr,
                                      trg_stride_ar,
                                      osp_type,
                                      scaling,
                                      osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPI_Recursive_PutAcc returned error\n");
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#endif /* OSPD_IMPLEMENTS_PUTACCV */
