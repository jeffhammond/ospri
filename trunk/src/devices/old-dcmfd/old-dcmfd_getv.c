/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

int OSPDI_Direct_getv(int target,
                     OSP_iov_t *iov_ar,
                     int ar_len,
                     OSPD_Handle_t *ospd_handle)
{
    int i, j, status = OSP_SUCCESS;
    size_t src_disp, dst_disp, size;
    DCMF_Callback_t done_callback;
    OSPD_Request_t *ospd_request;

    OSPU_FUNC_ENTER();

    for (i=0; i<ar_len; i++)
    {
        for(j=0; j<iov_ar[i].ptr_ar_len; j++)
        {

              src_disp = (size_t) iov_ar[i].source_ptr_ar[j] - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
              dst_disp = (size_t) iov_ar[i].target_ptr_ar[j] - (size_t) OSPD_Membase_global[target];
              size = iov_ar[i].size;

              ospd_request = OSPDI_Get_request(1);
              OSPU_ERR_POP(status = (ospd_request == NULL), "OSPDI_Get_request returned error.  \n");
              OSPDI_Set_handle(ospd_request, ospd_handle);

              done_callback.function = OSPDI_Request_done;
              done_callback.clientdata = (void *) ospd_request;

              ospd_handle->active++;

              status = DCMF_Get(&OSPD_Generic_get_protocol,
                                &(ospd_request->request),
                                done_callback,
                                DCMF_RELAXED_CONSISTENCY,
                                target,
                                size,
                                &OSPD_Memregion_global[target],
                                &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                                src_disp,
                                dst_disp);
              OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Get returned with an error \n");

        }
    } 

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_GetV(int target,
             OSP_iov_t *iov_ar,
             int ar_len)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = OSPDI_Get_handle();
    OSPU_ERR_POP(status = (ospd_handle == NULL), "OSPDI_Get_handle returned NULL in OSPD_GetV.\n");

    status = OSPDI_Direct_getv(target, iov_ar, ar_len, ospd_handle);
    OSPU_ERR_POP(status, "OSPDI_Direct_getv returned with an error \n");

    OSPDI_Conditional_advance(ospd_handle->active > 0);

  fn_exit:
    OSPDI_Release_handle(ospd_handle);
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_NbGetV(int target,
               OSP_iov_t *iov_ar,
               int ar_len,
               OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    status = OSPDI_Direct_getv(target, iov_ar, ar_len, ospd_handle);
    OSPU_ERR_POP(status, "OSPDI_Direct_getv returned with an error \n");

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
