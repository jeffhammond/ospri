/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospu.h"
#include "ospd.h"

#define OSPUI_MOD_BXOR(datatype, source, target, count)                             \
        do {                                                                       \
            int w;                                                                 \
            datatype *s = (datatype *) source;                                     \
            datatype *t = (datatype *) target;                                     \
            for(w=0; w<count; w++)                                                 \
            t[w] ^= s[w];                                                          \
        } while(0)                                                                 \

int OSPU_ModV_memcpy(OSP_iov_t *iov_ar,
                    int ar_len,
                    OSP_reduce_op_t osp_op,
                    OSP_datatype_t osp_type,
                    void* scaling)
{
    int i, j, status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPD_Global_lock_acquire();

    for (i=0; i<ar_len; i++)
    {
        for(j=0; j<iov_ar[i].ptr_ar_len; j++) 
        {
            switch (osp_op)
            {
                case OSP_BXOR:
                    switch (osp_type)
                    {
                        case OSP_INT32:
                            OSPUI_MOD_BXOR(int32_t,
                                     iov_ar[i].source_ptr_ar[j],
                                     iov_ar[i].target_ptr_ar[j],
                                     (iov_ar[i].size)/sizeof(int32_t));
                            break;
                        case OSP_INT64:
                            OSPUI_MOD_BXOR(int64_t,
                                     iov_ar[i].source_ptr_ar[j],
                                     iov_ar[i].target_ptr_ar[j],
                                     (iov_ar[i].size)/sizeof(int64_t));
                            break;
                        case OSP_UINT32:
                            OSPUI_MOD_BXOR(uint32_t,
                                     iov_ar[i].source_ptr_ar[j],
                                     iov_ar[i].target_ptr_ar[j],
                                     (iov_ar[i].size)/sizeof(uint32_t));
                            break;
                        case OSP_UINT64:
                            OSPUI_MOD_BXOR(uint64_t,
                                     iov_ar[i].source_ptr_ar[j],
                                     iov_ar[i].target_ptr_ar[j],
                                     (iov_ar[i].size)/sizeof(uint64_t));
                            break;
                        default:
                            status = OSP_ERROR;
                            OSPU_ERR_POP((status != OSP_SUCCESS), "Invalid data type in OSPU_AccV_memcpy\n");
                            break;
                    }
                    break;
                default:
                    status = OSP_ERROR;
                    OSPU_ERR_POP((status != OSP_SUCCESS), "Invalid op type in OSPU_AccV_memcpy\n");
                    break;
            }

        }
    }

    OSPD_Global_lock_release();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}
