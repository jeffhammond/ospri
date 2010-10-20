/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Generic_get_protocol;

int OSPDI_Get_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Get_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_GET_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    status = DCMF_Get_register(&OSPD_Generic_get_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "get registartion returned with error %d \n",
                status);

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSPD_Get(int target, void* src, void* dst, int bytes)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t callback;
    volatile int active;
    unsigned src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    callback.function = OSPDI_Generic_done;
    callback.clientdata = (void *) &active;

    src_disp = (size_t) src - (size_t) OSPD_Membase_global[target];
    dst_disp = (size_t) dst - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];

    active = 1;

    status = DCMF_Get(&OSPD_Generic_get_protocol,
                      &request,
                      callback,
                      DCMF_RELAXED_CONSISTENCY,
                      target,
                      bytes,
                      &OSPD_Memregion_global[target],
                      &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                      src_disp,
                      dst_disp);
    OSPU_ERR_POP(status, "DCMF_Get returned with an error \n");

    OSPDI_Conditional_advance(active > 0);

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}


int OSPD_NbGet(int target, void* src, void* dst, int bytes, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t* ospd_handle = NULL;
    OSPD_Request_t* ospd_request = NULL;
    DCMF_Callback_t callback;
    unsigned src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    ospd_handle->active++;

    ospd_request = OSPDI_Get_request(1);
    OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error.\n");
    OSPDI_Set_handle(ospd_request, ospd_handle); 

    callback.function = OSPDI_Request_done;
    callback.clientdata = (void *) ospd_request;

    src_disp = (size_t) src - (size_t) OSPD_Membase_global[target];
    dst_disp = (size_t) dst - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];

    status = DCMF_Get(&OSPD_Generic_get_protocol,
                      &(ospd_request->request),
                      callback,
                      DCMF_RELAXED_CONSISTENCY,
                      target,
                      bytes,
                      &OSPD_Memregion_global[target],
                      &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                      src_disp,
                      dst_disp);
    OSPU_ERR_POP(status, "DCMF_Get returned with an error \n");

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
