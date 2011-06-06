/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Generic_put_protocol;

int OSPDI_Put_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Put_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_PUT_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    status = DCMF_Put_register(&OSPD_Generic_put_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Put_register returned with error %d \n",
                status);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Put(int target,
            void* src,
            void* dst,
            int bytes)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t done_callback, ack_callback;
    volatile int done_active, ack_active;
    size_t src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    done_callback.function = OSPDI_Generic_done;
    done_callback.clientdata = (void *) &done_active;
    done_active = 1;
    OSPD_Connection_put_active[target]++;

    src_disp = (size_t) src - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
    dst_disp = (size_t) dst - (size_t) OSPD_Membase_global[target];

    status = DCMF_Put(&OSPD_Generic_put_protocol,
                      &request,
                      done_callback,
                      DCMF_SEQUENTIAL_CONSISTENCY,
                      target,
                      bytes,
                      &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                      &OSPD_Memregion_global[target],
                      src_disp,
                      dst_disp,
                      OSPD_Nocallback);
    OSPU_ERR_POP(status, "DCMF_Put returned with an error \n");

    OSPDI_Conditional_advance(done_active > 0);

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
   goto fn_exit;
}

int OSPD_NbPut(int target,
              void* src,
              void* dst,
              int bytes,
              OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;
    OSPD_Request_t *ospd_request;
    DCMF_Callback_t done_callback, ack_callback;
    size_t src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    ospd_request = OSPDI_Get_request(1);
    OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error \n");
    OSPDI_Set_handle(ospd_request, ospd_handle);

    done_callback.function = OSPDI_Request_done;
    done_callback.clientdata = (void *) ospd_request;

    ospd_handle->active++;

    src_disp = (size_t) src - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
    dst_disp = (size_t) dst - (size_t) OSPD_Membase_global[target];

    status = DCMF_Put(&OSPD_Generic_put_protocol,
                      &(ospd_request->request),
                      done_callback,
                      DCMF_SEQUENTIAL_CONSISTENCY,
                      target,
                      bytes,
                      &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                      &OSPD_Memregion_global[target],
                      src_disp,
                      dst_disp,
                      OSPD_Nocallback);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Put returned with an error \n");

    OSPD_Connection_put_active[target]++;

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}
