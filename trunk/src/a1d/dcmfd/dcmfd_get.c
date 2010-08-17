/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t A1D_Generic_get_protocol;

DCMF_Result A1DI_Get_initialize()
{
    DCMF_Result result = DCMF_SUCCESS;
    DCMF_Get_Configuration_t conf;

    A1U_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_GET_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    result = DCMF_Get_register(&A1D_Generic_get_protocol, &conf);
    A1U_ERR_POP(result != DCMF_SUCCESS,
                "get registartion returned with error %d \n",
                result);

  fn_exit:
    A1U_FUNC_EXIT();
    return result;

  fn_fail:
    goto fn_exit;
}

int A1D_Get(int target, void* src, void* dst, int bytes)
{
    DCMF_Result result = DCMF_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t callback;
    volatile int active;
    unsigned src_disp, dst_disp;

    A1U_FUNC_ENTER();

    A1DI_CRITICAL_ENTER();

    callback.function = A1DI_Generic_done;
    callback.clientdata = (void *) &active;

    src_disp = (size_t) src - (size_t) A1D_Membase_global[target];
    dst_disp = (size_t) dst - (size_t) A1D_Membase_global[A1D_Process_info.my_rank];

    active = 1;

    result = DCMF_Get(&A1D_Generic_get_protocol,
                      &request,
                      callback,
                      DCMF_RELAXED_CONSISTENCY,
                      target,
                      bytes,
                      &A1D_Memregion_global[target],
                      &A1D_Memregion_global[A1D_Process_info.my_rank],
                      src_disp,
                      dst_disp);
    A1U_ERR_POP(result, "DCMF_Get returned with an error \n");

    A1DI_Conditional_advance(active > 0);

  fn_exit: 
    A1DI_CRITICAL_EXIT();
    A1U_FUNC_EXIT();
    return result;

  fn_fail: 
    goto fn_exit;
}


int A1D_NbGet(int target, void* src, void* dst, int bytes, A1_handle_t* a1_handle)
{
    DCMF_Result result = DCMF_SUCCESS;
    A1D_Handle_t* a1d_handle;
    DCMF_Callback_t callback;
    unsigned src_disp, dst_disp;

    A1U_FUNC_ENTER();

    A1DI_CRITICAL_ENTER();

    /* Initializing handle. the handle must have been initialized using *
     * A1_Init_handle */
    if(a1_handle == NULL)
    {
      a1d_handle = A1DI_Get_handle();
      A1DI_Load_request(a1d_handle);
      A1DI_Set_user_handle(a1d_handle, a1_handle);
      *a1_handle = (A1_handle_t) a1d_handle;
    }
    else
    {
      a1d_handle = (A1D_handle_t) a1_handle;
      A1DI_Load_request(a1d_handle);
    }

    callback.function = A1DI_Handle_done;
    callback.clientdata = (void *) a1d_handle;
    a1d_handle->active++;

    src_disp = (size_t) src - (size_t) A1D_Membase_global[target];
    dst_disp = (size_t) dst - (size_t) A1D_Membase_global[A1D_Process_info.my_rank];

    result = DCMF_Get(&A1D_Generic_get_protocol,
                      &(a1d_handle->request_list->request),
                      callback,
                      DCMF_RELAXED_CONSISTENCY,
                      target,
                      bytes,
                      &A1D_Memregion_global[target],
                      &A1D_Memregion_global[A1D_Process_info.my_rank],
                      src_disp,
                      dst_disp);
    A1U_ERR_POP(result, "DCMF_Get returned with an error \n");

  fn_exit: 
    A1DI_CRITICAL_EXIT();
    A1U_FUNC_EXIT();
    return result;

  fn_fail: 
    goto fn_exit;
}
