/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospu.h"
#include "ospd.h"

int OSPU_Get_memcpy(void* src,
                   void* dst,
                   int bytes)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPD_Global_lock_acquire();

    memcpy(dst, src, bytes);

    OSPD_Global_lock_release();

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSPU_GetS_memcpy(int stride_level,
                    int *block_sizes,
                    void* source_ptr,
                    int *src_stride_ar,
                    void* target_ptr,
                    int *trg_stride_ar)
{
    int status = OSP_SUCCESS;
    int chunk_count = 1;
    int *block_sizes_w;
    int i, y;

    OSPU_FUNC_ENTER();

    OSPD_Global_lock_acquire();

    block_sizes_w = malloc(sizeof(int) * (stride_level + 1));
    OSPU_ERR_POP((status = (NULL == block_sizes_w)),
                "malloc failed in OSPU_GetS_memcpy");

    memcpy(block_sizes_w, block_sizes, sizeof(int) * (stride_level + 1));

    for (i = 1; i <= stride_level; i++)
        chunk_count = block_sizes[i] * chunk_count;

    for (i = 0; i < chunk_count; i++)
    {
        memcpy(target_ptr, source_ptr, block_sizes[0]);

        block_sizes_w[1]--;
        if (block_sizes_w[1] == 0)
        {
            y = 1;
            while (block_sizes_w[y] == 0)
            {
                if (y == stride_level)
                {
                    OSPU_ASSERT(i == chunk_count - 1, status);
                    return status;
                }
                y++;
            }
            block_sizes_w[y]--;

            /*The strides done on lower dimensions should be subtracted as these are
              included in the stride along the current dimension*/
            source_ptr = (void *) ((size_t) source_ptr 
                    + src_stride_ar[y - 1]
                                    - (block_sizes[y-1] - 1) * src_stride_ar[y-2]);
            target_ptr = (void *) ((size_t) target_ptr 
                    + trg_stride_ar[y - 1]
                                    - (block_sizes[y-1] - 1) * trg_stride_ar[y-2]);

            y--;
            while (y >= 1)
            {
                block_sizes_w[y] = block_sizes[y];
                y--;
            }
        }
        else
        {
            source_ptr = (void *) ((size_t) source_ptr + src_stride_ar[0]);
            target_ptr = (void *) ((size_t) target_ptr + trg_stride_ar[0]);
        }
    }

    OSPD_Global_lock_release();

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSPU_GetV_memcpy(OSP_iov_t *iov_ar, int ar_len)
{
    int i, j, status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPD_Global_lock_acquire();

    for (i=0; i<ar_len; i++)
    {
        for(j=0; j<iov_ar[i].ptr_ar_len; j++) 
        {
            memcpy(iov_ar[i].target_ptr_ar[j], iov_ar[i].source_ptr_ar[j], iov_ar[i].size);
        }
    }

    OSPD_Global_lock_release();

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}
