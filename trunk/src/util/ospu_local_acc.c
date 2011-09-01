/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospu.h"
#include "ospd.h"

#define OSPUI_ACC(datatype, source, target, scaling, count)                  \
        do {                                                                     \
            unsigned w;                                                                 \
            datatype *s = (datatype *) source;                                     \
            datatype *t = (datatype *) target;                                     \
            datatype c = (datatype) scaling;                                       \
            for(w=0; w<count; w++)                                                 \
            t[w] += s[w]*c;                                                   \
        } while(0)                                                               \

int OSPU_Acc_memcpy(void* source_ptr,
        void* target_ptr,
        unsigned bytes,
        osp_datatype_t osp_type,
        void* scaling)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    switch (osp_type)
    {
    case OSP_DOUBLE:
        OSPUI_ACC(double,
                source_ptr,
                target_ptr,
                *((double *) scaling),
                bytes/sizeof(double));
    case OSP_INT32:
        OSPUI_ACC(int32_t,
                source_ptr,
                target_ptr,
                *((int32_t *) scaling),
                bytes/sizeof(int32_t));
        break;
    case OSP_INT64:
        OSPUI_ACC(int64_t,
                source_ptr,
                target_ptr,
                *((int64_t *) scaling),
                bytes/sizeof(int64_t));
        break;
    case OSP_UINT32:
        OSPUI_ACC(uint32_t,
                source_ptr,
                target_ptr,
                *((uint32_t *) scaling),
                bytes/sizeof(uint32_t));
        break;
    case OSP_UINT64:
        OSPUI_ACC(uint64_t,
                source_ptr,
                target_ptr,
                *((uint64_t *) scaling),
                bytes/sizeof(uint64_t));
        break;
    case OSP_FLOAT:
        OSPUI_ACC(float,
                source_ptr,
                target_ptr,
                *((float *) scaling),
                bytes/sizeof(float));
        break;
    default:
        status = OSP_ERROR;
        OSPU_ERR_POP((status != OSP_SUCCESS), "Invalid datatype in OSPU_Acc_memcpy \n");
        break;
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSPU_AccS_memcpy(int stride_level,
        int *block_sizes,
        void* source_ptr,
        int *src_stride_ar,
        void* target_ptr,
        int *trg_stride_ar,
        osp_datatype_t osp_type,
        void* scaling)
{
    int status = OSP_SUCCESS;
    int chunk_count = 1;
    int *block_sizes_w;
    int i, y;

    OSPU_FUNC_ENTER();

    block_sizes_w = malloc(sizeof(int) * (stride_level + 1));
    OSPU_ERR_POP((status = (NULL == block_sizes_w)),
            "malloc failed in OSPU_PutS_memcpy");

    memcpy(block_sizes_w, block_sizes, sizeof(int) * (stride_level + 1));

    for (i = 1; i <= stride_level; i++)
        chunk_count = block_sizes[i] * chunk_count;

    for (i = 0; i < chunk_count; i++)
    {
        switch (osp_type)
        {
            case OSP_DOUBLE:
                OSPUI_ACC(double,
                        source_ptr,
                        target_ptr,
                        *((double *) scaling),
                        block_sizes[0]/sizeof(double));
                break;
            case OSP_INT32:
                OSPUI_ACC(int32_t,
                        source_ptr,
                        target_ptr,
                        *((int32_t *) scaling),
                        block_sizes[0]/sizeof(int32_t));
                break;
            case OSP_INT64:
                OSPUI_ACC(int64_t,
                        source_ptr,
                        target_ptr,
                        *((int64_t *) scaling),
                        block_sizes[0]/sizeof(int64_t));
                break;
            case OSP_UINT32:
                OSPUI_ACC(uint32_t,
                        source_ptr,
                        target_ptr,
                        *((uint32_t *) scaling),
                        block_sizes[0]/sizeof(uint32_t));
                break;
            case OSP_UINT64:
                OSPUI_ACC(uint64_t,
                        source_ptr,
                        target_ptr,
                        *((uint64_t *) scaling),
                        block_sizes[0]/sizeof(uint64_t));
                break;
            case OSP_FLOAT:
                OSPUI_ACC(float,
                        source_ptr,
                        target_ptr,
                        *((float *) scaling),
                        block_sizes[0]/sizeof(float));
                break;
            default:
                status = OSP_ERROR;
                OSPU_ERR_POP((status != OSP_SUCCESS), "Invalid data type in putacc \n");
                break;
        }

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

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

