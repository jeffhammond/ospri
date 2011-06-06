/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

int OSPDI_Direct_putaccv(int target,
                        OSP_iov_t *iov_ar,
                        int ar_len,
                        OSP_datatype_t osp_type,
                        void *scaling,
                        OSPD_Handle_t *ospd_handle)
{
    int i, j, status = OSP_SUCCESS;
    OSPD_Putacc_header_t header;
    OSPD_Request_t *ospd_request;
    DCMF_Callback_t done_callback;

    OSPU_FUNC_ENTER();

    header.datatype = osp_type;
    switch (osp_type)
    {
        case OSP_DOUBLE:
            (header.scaling).double_value = *((double *) scaling);
            break;
        case OSP_INT32:
            (header.scaling).int32_value = *((int32_t *) scaling);
            break;
        case OSP_INT64:
            (header.scaling).int64_value = *((int64_t *) scaling);
            break;
        case OSP_UINT32:
            (header.scaling).uint32_value = *((uint32_t *) scaling);
            break;
        case OSP_UINT64:
            (header.scaling).uint64_value = *((uint64_t *) scaling);
            break;
        case OSP_FLOAT:
            (header.scaling).float_value = *((float *) scaling);
            break;
        default:
            status = OSP_ERROR;
            OSPU_ERR_POP((status != OSP_SUCCESS),"Invalid data type in putacc \n");
            break;
    }

    for (i=0; i<ar_len; i++)
    {
        for(j=0; j<iov_ar[i].ptr_ar_len; j++)
        {

           ospd_request = OSPDI_Get_request(1);
           OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error.\n");
           OSPDI_Set_handle(ospd_request, ospd_handle);

           done_callback.function = OSPDI_Request_done;
           done_callback.clientdata = (void *) ospd_request;
 
           ospd_handle->active++;

           header.target_ptr = iov_ar[i].target_ptr_ar[j];
 
           status = DCMF_Send(&OSPD_Generic_putacc_protocol,
                              &(ospd_request->request),
                              done_callback,
                              DCMF_SEQUENTIAL_CONSISTENCY,
                              target,
                              iov_ar[i].size,
                              iov_ar[i].source_ptr_ar[j],
                              (DCQuad *) &header,
                              (unsigned) 2);
           OSPU_ERR_POP((status != DCMF_SUCCESS), "Putacc returned with an error \n");
 
           OSPD_Connection_send_active[target]++;
        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_PutAccV(int target,
                OSP_iov_t *iov_ar,
                int ar_len,
                OSP_datatype_t osp_type,
                void* scaling)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = OSPDI_Get_handle();
    OSPU_ERR_POP(status = (ospd_handle == NULL),
                "OSPDI_Get_handle returned NULL in OSPD_PutAccS.\n");

    status = OSPDI_Direct_putaccv(target,
                                 iov_ar,
                                 ar_len,
                                 osp_type,
                                 scaling,
                                 ospd_handle);
    OSPU_ERR_POP(status, "Direct putaccv function returned with an error \n");

    OSPDI_Conditional_advance(ospd_handle->active > 0);

  fn_exit:
    OSPDI_Release_handle(ospd_handle);
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_NbPutAccV(int target,
                  OSP_iov_t *iov_ar,
                  int ar_len,
                  OSP_datatype_t osp_type,
                  void* scaling,
                  OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    status = OSPDI_Direct_putaccv(target,
                                 iov_ar,
                                 ar_len,
                                 osp_type,
                                 scaling,
                                 ospd_handle);
    OSPU_ERR_POP(status, "Direct putaccv function returned with an error \n");

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
