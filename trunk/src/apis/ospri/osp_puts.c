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
#define OSPD_IMPLEMENTS_PUTS

#if defined OSPD_IMPLEMENTS_PUTS

int OSP_PutS(int target,
            int stride_level,
            int *block_sizes,
            void* source_ptr,
            int *src_stride_ar,
            void* target_ptr,
            int *trg_stride_ar)
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
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUTS, target, total_bytes);
    }
#   endif
   
    /* Check if it is a contiguous transfer, issue a contiguous op*/
    if(stride_level == 0)
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_1d) )
        {
           status = OSPU_Put_memcpy(source_ptr, target_ptr, block_sizes[0]);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Put_memcpy returned an error\n");
        }
        else
        {
           status = OSPD_Put(target, source_ptr, target_ptr, block_sizes[0]);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Put returned an error\n");        
        }
        goto fn_exit;
    }
    else /* Non-contiguous */
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_Nd) )
        {
            status = OSPU_PutS_memcpy(stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_PutS_memcpy returned error\n");
        }
        else
        {
            status = OSPD_PutS(target,
                              stride_level,
                              block_sizes,
                              source_ptr,
                              src_stride_ar,
                              target_ptr,
                              trg_stride_ar);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_PutS returned error\n");
        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}


int OSP_NbPutS(int target,
              int stride_level,
              int *block_sizes,
              void* source_ptr,
              int *src_stride_ar,
              void* target_ptr,
              int *trg_stride_ar,
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
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBPUTS, target, total_bytes);
    }
#   endif

    /*Check if it is a contiguous transfer, issue a contiguous op*/
    if(stride_level == 0)
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_1d) )
        {
           status = OSPU_Put_memcpy(source_ptr, target_ptr, block_sizes[0]);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Put_memcpy returned an error\n");
        }
        else
        {
           status = OSPD_NbPut(target, source_ptr, target_ptr, block_sizes[0], osp_handle);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbPut returned an error\n");
        }
        goto fn_exit;
    }
    else /* Non-contiguous */
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_Nd) )
        {
            status = OSPU_PutS_memcpy(stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_PutS_memcpy returned error\n");
        }
        else
        {
            status = OSPD_NbPutS(target,
                                stride_level,
                                block_sizes,
                                source_ptr,
                                src_stride_ar,
                                target_ptr,
                                trg_stride_ar,
                                osp_handle);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbPutS returned error\n");
        }
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

#else

int OSPI_Recursive_Put(int target,
                      int stride_level,
                      int *block_sizes,
                      void *source_ptr,
                      int *src_stride_ar,
                      void *target_ptr,
                      int *trg_stride_ar,
                      OSP_handle_t osp_handle)
{
    int i, status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    if (stride_level > 0)
    {
        for (i = 0; i < block_sizes[stride_level]; i++)
        {
            status = OSPI_Recursive_Put(target,
                                       stride_level - 1,
                                       block_sizes,
                                       (void *) ((size_t) source_ptr + i * src_stride_ar[stride_level - 1]),
                                       src_stride_ar,
                                       (void *) ((size_t) target_ptr + i * trg_stride_ar[stride_level - 1]),
                                       trg_stride_ar,
                                       osp_handle);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                        "OSPI_Recursive_Put returned error in OSPI_Recursive_Put.\n");
        }
    }
    else
    {
        status = OSPD_NbPut(target,
                           source_ptr,
                           target_ptr,
                           src_disp,
                           block_sizes[0],
                           osp_handle);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbPut returned with an error \n");
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_PutS(int target,
            int stride_level,
            int *block_sizes,
            void* source_ptr,
            int *src_stride_ar,
            void* target_ptr,
            int *trg_stride_ar)
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
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUTS, target, total_bytes);
    }
#   endif

    /*Check if it is a contiguous transfer, issue a contiguous op*/
    if(stride_level == 0)
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_1d) )
        {
           status = OSPU_Put_memcpy(source_ptr, target_ptr, block_sizes[0]);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Put_memcpy returned an error\n");
        }
        else
        {
           status = OSPD_Put(target, source_ptr, target_ptr, block_sizes[0]);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Put returned an error\n");
        }
        goto fn_exit;
    }
    else /* Non-contiguous */
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_Nd) )
        {
            status = OSPU_PutS_memcpy(stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_PutS_memcpy returned error\n");
        }
        else
        {
            status = OSPD_Allocate_handle(&osp_handle);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Allocate_handle returned error\n");

            status = OSPI_Recursive_Put(target,
                                       stride_level,
                                       block_sizes,
                                       source_ptr,
                                       src_stride_ar,
                                       target_ptr,
                                       trg_stride_ar,
                                       osp_handle);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Recursive_Put returned error\n");

            status = OSPD_Wait_handle(osp_handle);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Wait_handle returned error\n");
        }
    }

    fn_exit:
    /* Could also test for NULL, assuming we set it as such in the declaration. */
    if(target == my_rank && ospu_settings.network_bypass) OSPD_Release_handle(osp_handle);
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_NbPutS(int target,
              int stride_level,
              int *block_sizes,
              void* source_ptr,
              int *src_stride_ar,
              void* target_ptr,
              int *trg_stride_ar,
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
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBPUTS, target, total_bytes);
    }
#   endif

    /*Check if it is a contiguous transfer, issue a contiguous op*/
    if(stride_level == 0)
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_1d) )
        {
           status = OSPU_Put_memcpy(source_ptr, target_ptr, block_sizes[0]);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Put_memcpy returned an error\n");
        }
        else
        {
           status = OSPD_NbPut(target, source_ptr, target_ptr, block_sizes[0], osp_handle);
           OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbPut returned an error\n");
        }
        goto fn_exit;
    }
    else /* Non-contiguous */
    {
        if(target == my_rank && (block_sizes[0] < ospu_settings.network_bypass_upper_limit_Nd) )
        {
            status = OSPU_PutS_memcpy(stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_PutS_memcpy returned error\n");
        }
        else
        {
            status = OSPI_Recursive_Put(target,
                                       stride_level,
                                       block_sizes,
                                       source_ptr,
                                       src_stride_ar,
                                       target_ptr,
                                       trg_stride_ar,
                                       osp_handle);
            OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Recursive_Put returned error\n");
        }
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#endif /* OSPD_IMPLEMENTS_PUTS */
