/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

OSPD_Control_xchange_info_t OSPD_Control_xchange_info;

/*************************************************************
 Local Completion Callbacks
 **************************************************************/

void OSPDI_Generic_done(void *clientdata, DCMF_Error_t *error)
{
    --(*((uint32_t *) clientdata));
}

void OSPDI_Request_done(void *clientdata, DCMF_Error_t *error)
{
    OSPDI_Release_request((OSPD_Request_t *) clientdata);
}

/*************************************************************
 Control Protocol for Information Exchange
 **************************************************************/

void OSPDI_Control_xchange_callback(void *clientdata,
                                   const DCMF_Control_t *info,
                                   size_t peer)
{
    OSPDI_Memcpy((void *) ((size_t) OSPD_Control_xchange_info.xchange_ptr
                   + (size_t)(peer * OSPD_Control_xchange_info.xchange_size)),
                (void *) info,
                OSPD_Control_xchange_info.xchange_size);

    --(*((uint32_t *) clientdata));
}

int OSPDI_Control_xchange_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Control_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_CONTROL_PROTOCOL;
    conf.network = DCMF_DEFAULT_NETWORK;
    conf.cb_recv = OSPDI_Control_xchange_callback;
    conf.cb_recv_clientdata = (void *) &OSPD_Control_xchange_info.rcv_active;

    status = DCMF_Control_register(&OSPD_Control_xchange_info.protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "Control xchange registartion returned with error %d \n",
                status);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

/*************************************************************
 Data Packing Code
 **************************************************************/

int OSPDI_Pack_strided(void *data_ptr,
                      int data_limit,
                      int stride_level,
                      int *block_sizes,
                      void **source_ptr,
                      int *src_stride_ar,
                      void **target_ptr,
                      int *trg_stride_ar,
                      int *block_idx,
                      int *data_size,
                      int *complete)
{
    int status = OSP_SUCCESS;
    int y, index, size_data;
    int block_sizes_w[OSPC_MAX_STRIDED_DIM];

    OSPU_FUNC_ENTER();

    *complete = 0;
    *data_size = 0;

    while ((*data_size + block_sizes[0]) <= data_limit)
    {

        OSPDI_Memcpy(data_ptr, *source_ptr, block_sizes[0]);
        data_ptr = (void *) ((size_t) data_ptr + block_sizes[0]);
        *data_size = *data_size + block_sizes[0];

        block_idx[1]++;
        if (block_idx[1] == block_sizes[1])
        {
            y = 1;
            while (block_idx[y] == block_sizes[y])
            {
                if (y == stride_level)
                {
                    *complete = 1;
                    return status;
                }
                y++;
            }
            block_idx[y]++;

            /*The strides done on lower dimension should be subtracted as these are
              included in the stride along the current dimension*/ 
            *source_ptr = (void *) ((size_t) *source_ptr
                    + src_stride_ar[y - 1] 
                    - (block_sizes[y-1] - 1) * src_stride_ar[y-2]);
            *target_ptr = (void *) ((size_t) *target_ptr
                    + trg_stride_ar[y - 1] 
                    - (block_sizes[y-1] - 1) * trg_stride_ar[y-2]);

            y--;
            while (y >= 1)
            {
                block_idx[y] = 0;
                y--;
            }
        }
        else
        {
            *source_ptr = (void *) ((size_t) *source_ptr + src_stride_ar[0]);
            *target_ptr = (void *) ((size_t) *target_ptr + trg_stride_ar[0]);
        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPDI_Unpack_strided(void *data_ptr,
                        int data_size,
                        int stride_level,
                        int *block_sizes,
                        void *target_ptr,
                        int *trg_stride_ar,
                        int *block_idx,
                        int *complete)
{
    int status = OSP_SUCCESS;
    int y, index;

    OSPU_FUNC_ENTER();

    while (data_size > 0)
    {
        OSPDI_Memcpy(target_ptr, data_ptr, block_sizes[0]);

        data_ptr = (void *) ((size_t) data_ptr + block_sizes[0]);
        data_size = data_size - block_sizes[0];

        block_idx[1]++;
        if (block_idx[1] == block_sizes[1])
        {
            y = 1;
            while (block_idx[y] == block_sizes[y])
            {
                if (y == stride_level)
                {
                    *complete = 1;
                    break;
                }
                y++;
            }
            block_idx[y]++;

            /*The strides done on lower dimension should be subtracted as these are
              included in the stride along the current dimension*/
            target_ptr = (void *) ((size_t) target_ptr 
                   + trg_stride_ar[y - 1] 
                   - (block_sizes[y-1] - 1) * trg_stride_ar[y-2]);

            y--;
            while (y >= 1)
            {
                block_idx[y] = 0;
                y--;
            }
        }
        else
        {
            target_ptr = (void *) ((size_t) target_ptr + trg_stride_ar[0]);
        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPDI_Unpack_strided_acc(void *data_ptr,
                            int data_size,
                            int stride_level,
                            int *block_sizes,
                            void *target_ptr,
                            int *trg_stride_ar,
                            int *block_idx,
                            OSP_datatype_t osp_type,
                            void *scaling,
                            int *complete)
{
    int status = OSP_SUCCESS;
    int y;

    OSPU_FUNC_ENTER();

    while (data_size > 0)
    {
        likely_if(osp_type == OSP_DOUBLE)
        {
            OSPDI_ACC(double,
                    data_ptr,
                    target_ptr,
                    *((double *) scaling),
                    block_sizes[0]/sizeof(double));
        }
        else
        {
            switch (osp_type)
            {
                case OSP_INT32:
                OSPDI_ACC(int32_t,
                        data_ptr,
                        target_ptr,
                        *((int32_t *) scaling),
                        block_sizes[0] / sizeof(int32_t));
                break;
                case OSP_INT64:
                OSPDI_ACC(int64_t,
                        data_ptr,
                        target_ptr,
                        *((int64_t *) scaling),
                        block_sizes[0] / sizeof(int64_t));
                break;
                case OSP_UINT32:
                OSPDI_ACC(uint32_t,
                        data_ptr,
                        target_ptr,
                        *((uint32_t *) scaling),
                        block_sizes[0] / sizeof(uint32_t));
                break;
                case OSP_UINT64:
                OSPDI_ACC(uint64_t,
                        data_ptr,
                        target_ptr,
                        *((uint64_t *) scaling),
                        block_sizes[0] / sizeof(uint64_t));
                break;
                case OSP_FLOAT:
                OSPDI_ACC(float,
                        data_ptr,
                        target_ptr,
                        *((float *) scaling),
                        block_sizes[0]/sizeof(float));
                break;
                default:
                OSPU_ERR_ABORT(OSP_ERROR,
                        "Invalid datatype received in Putacc operation \n");
                break;
            }
        }

        data_ptr = (void *) ((size_t) data_ptr + block_sizes[0]);
        data_size = data_size - block_sizes[0];

        block_idx[1]++;
        if (block_idx[1] == block_sizes[1])
        {
            y = 1;
            while (block_idx[y] == block_sizes[y])
            {
                if (y == stride_level)
                {
                    *complete = 1;
                    break;
                }
                y++;
            }
            block_idx[y]++;

            /*The strides done on lower dimension should be subtracted as these are
              included in the stride along the current dimension*/
            target_ptr = (void *) ((size_t) target_ptr 
                   + trg_stride_ar[y - 1] 
                   - (block_sizes[y-1] - 1) * trg_stride_ar[y-2]);

            y--;
            while (y >= 1)
            {
                block_idx[y] = 0;
                y--;
            }
        }
        else
        {
            target_ptr = (void *) ((size_t) target_ptr + trg_stride_ar[0]);
        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

/**** Exposing locking to ADL layer ****/

void OSPD_Global_lock_acquire()
{
    OSPU_FUNC_ENTER();

    OSPDI_GLOBAL_LOCK_ACQUIRE();

  fn_exit:
    OSPU_FUNC_EXIT();
    return;

  fn_fail:
    goto fn_exit;
}

void OSPD_Global_lock_release()
{
    OSPU_FUNC_ENTER();

    OSPDI_GLOBAL_LOCK_RELEASE();

  fn_exit:
    OSPU_FUNC_EXIT();
    return;

  fn_fail:
    goto fn_exit;
}
