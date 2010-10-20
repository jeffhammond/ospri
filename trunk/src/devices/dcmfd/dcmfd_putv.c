/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

int OSPDI_Direct_putv(int target,
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

           src_disp = (size_t) iov_ar[i].source_ptr_ar[j]
                   - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
           dst_disp = (size_t) iov_ar[i].target_ptr_ar[j]
                   - (size_t) OSPD_Membase_global[target];
           size = iov_ar[i].size;

           ospd_request = OSPDI_Get_request(1);
           OSPU_ERR_POP(status = (ospd_request == NULL),
               "OSPDI_Get_request returned error. \n");
           OSPDI_Set_handle(ospd_request, ospd_handle);

           done_callback.function = OSPDI_Request_done;
           done_callback.clientdata = (void *) ospd_request;

           ospd_handle->active++;

           status = DCMF_Put(&OSPD_Generic_put_protocol,
                          &(ospd_request->request),
                          done_callback,
                          DCMF_SEQUENTIAL_CONSISTENCY,
                          target,
                          size,
                          &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                          &OSPD_Memregion_global[target],
                          src_disp,
                          dst_disp,
                          OSPD_Nocallback);
            OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Put returned with an error \n");

            OSPD_Connection_put_active[target]++;

        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_PutV(int target,
             OSP_iov_t *iov_ar,
             int ar_len)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = OSPDI_Get_handle();
    OSPU_ERR_POP(status = (ospd_handle == NULL),
                "OSPDI_Get_handle returned NULL in OSPD_PutS. \n");

    status = OSPDI_Direct_putv(target,
                              iov_ar,
                              ar_len,
                              ospd_handle); 
    OSPU_ERR_POP(status, "OSPDI_Direct_putv returned with an error \n");

    OSPDI_Conditional_advance(ospd_handle->active > 0);

  fn_exit:
    OSPDI_Release_handle(ospd_handle);
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_NbPutV(int target,
               OSP_iov_t *iov_ar,
               int ar_len,
               OSP_handle_t osp_handle)
{   
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    status = OSPDI_Direct_putv(target,
                              iov_ar,
                              ar_len,
                              ospd_handle);    
    OSPU_ERR_POP(status, "OSPDI_Direct_putv returned with an error \n");

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
