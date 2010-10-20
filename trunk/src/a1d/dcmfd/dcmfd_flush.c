/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Send_flush_protocol;
DCMF_Protocol_t OSPD_Control_flushack_protocol;
void **OSPD_Put_Flushcounter_ptr;
volatile int OSPD_Control_flushack_active;
volatile int OSPD_Put_flushack_active;
volatile int *OSPD_Connection_send_active;
volatile int *OSPD_Connection_put_active;

/**************************************************************** 
 * Control protocol used to send acknowledgements to send-flush *
 * messages                                                     *
 ****************************************************************/

/* TODO: If this is just decrementing the payload like OSPDI_Generic_done, why don't we have
 * another OSPD_Generic_control_done?  Better yet, call them OSPDI_Send_decrement and
 * OSPD_Control_decrement. */
void OSPDI_Control_flushack_callback(void *clientdata,
                                    const DCMF_Control_t *info,
                                    size_t peer)
{
    --(*((uint32_t *) clientdata));

}

int OSPDI_Control_flushack_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Control_Configuration_t conf;

    OSPU_FUNC_ENTER();

    OSPD_Control_flushack_active = 0;

    conf.protocol = DCMF_DEFAULT_CONTROL_PROTOCOL;
    conf.network = DCMF_DEFAULT_NETWORK;
    conf.cb_recv = OSPDI_Control_flushack_callback;
    conf.cb_recv_clientdata = (void *) &OSPD_Control_flushack_active;

    status = DCMF_Control_register(&OSPD_Control_flushack_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "Control flushack registartion returned with error %d \n",
                status);

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

/****************************************************************
 * Send protocol used to flush send message                     *
 ****************************************************************/

void OSPDI_RecvSendShort_flush_callback(void *clientdata,
                                       const DCQuad *msginfo,
                                       unsigned count,
                                       size_t peer,
                                       const char *src,
                                       size_t bytes)
{
    int status = OSP_SUCCESS;
    DCMF_Control_t info;

    status = DCMF_Control(&OSPD_Control_flushack_protocol,
                          DCMF_SEQUENTIAL_CONSISTENCY,
                          peer,
                          &info);
    OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                  "DCMF_Control failed in OSPDI_RecvSendShort_flush_callback\n");

}

int OSPDI_Send_flush_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Send_Configuration_t conf;

    OSPU_FUNC_ENTER();

    /* FIXME: The recv callback should be implemented when Send might be used *
     * with large messages */

    conf.protocol = DCMF_DEFAULT_SEND_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    conf.cb_recv_short = OSPDI_RecvSendShort_flush_callback;
    conf.cb_recv_short_clientdata = NULL;
    conf.cb_recv = NULL;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Send_register(&OSPD_Send_flush_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Send_register failed with error %d \n",
                status);

    /* Allocating memory for vector that tracks connections with active sends */
    status = OSPDI_Malloc((void **) &OSPD_Connection_send_active,
                                 sizeof(int) * OSPD_Process_info.num_ranks);
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc failed \n");
    OSPDI_Memset((void *) OSPD_Connection_send_active,
                0,
                sizeof(int) * OSPD_Process_info.num_ranks);

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

/****************************************************************
 * Put protocol used to flush put messages                      *
 ****************************************************************/

int OSPDI_Put_flush_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Control_t info;
    int rank;

    OSPU_FUNC_ENTER();

    status = OSPDI_Malloc((void **) &OSPD_Put_Flushcounter_ptr,
                                 sizeof(void *) * OSPD_Process_info.num_ranks);
    OSPU_ERR_POP(status != 0, "OSPDI_Malloc failed \n");

    status = OSPDI_Malloc((void **) &(OSPD_Put_Flushcounter_ptr[OSPD_Process_info.my_rank]),
                                  2);
    OSPU_ERR_POP(status != 0, "OSPDI_Malloc failed \n");

    /*TODO: Use DCMF_Send operations instead to exploit TORUS network */
    OSPD_Control_xchange_info.xchange_ptr = (void *) OSPD_Put_Flushcounter_ptr;
    OSPD_Control_xchange_info.xchange_size = sizeof(void *);
    OSPD_Control_xchange_info.rcv_active += OSPD_Process_info.num_ranks - 1;

    OSPDI_GlobalBarrier();

    OSPDI_Memcpy((void *) &info,
                (void *) &(OSPD_Put_Flushcounter_ptr[OSPD_Process_info.my_rank]),
                sizeof(void *));
    for (rank = 0; rank < OSPD_Process_info.num_ranks; rank++)
    {
        likely_if (rank != OSPD_Process_info.my_rank)
        {
            status = DCMF_Control(&OSPD_Control_xchange_info.protocol,
                                  DCMF_SEQUENTIAL_CONSISTENCY,
                                  rank,
                                  &info);
            OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Control failed\n");
        }
    }
    OSPDI_Conditional_advance(OSPD_Control_xchange_info.rcv_active > 0);

    /* Allocating memory for vector thats tracks connections with active puts */
    status = OSPDI_Malloc((void **) &OSPD_Connection_put_active,
                                 sizeof(int) * OSPD_Process_info.num_ranks);
    OSPU_ERR_POP(status != 0,
                "Connection put active buffer allocation Failed \n");
    OSPDI_Memset((void *) OSPD_Connection_put_active,
                0,
                sizeof(int) * OSPD_Process_info.num_ranks);

    OSPD_Put_flushack_active = 0;

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;

}

int OSPDI_Send_flush(int proc)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCQuad msginfo;

    OSPU_FUNC_ENTER();

    OSPD_Control_flushack_active++;

    status = DCMF_Send(&OSPD_Send_flush_protocol,
                       &request,
                       OSPD_Nocallback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       proc,
                       0,
                       NULL,
                       &msginfo,
                       1);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Send returned with an error \n");

    OSPDI_Conditional_advance(OSPD_Control_flushack_active > 0);

    OSPD_Connection_send_active[proc] = 0;
    OSPD_Connection_put_active[proc] = 0;

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPDI_Send_flush_start(int proc)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t callback;
    DCQuad msginfo;

    OSPU_FUNC_ENTER();

    OSPD_Control_flushack_active++;

    status = DCMF_Send(&OSPD_Send_flush_protocol,
                       &request,
                       OSPD_Nocallback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       proc,
                       0,
                       NULL,
                       &msginfo,
                       1);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Send returned with an error \n");

    OSPD_Connection_send_active[proc] = 0;
    OSPD_Connection_put_active[proc] = 0;

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPDI_Put_flush(int proc)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t callback;
    volatile int active;
    size_t src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    callback.function = OSPDI_Generic_done;
    callback.clientdata = (void *) &OSPD_Put_flushack_active;

    src_disp = (size_t)(OSPD_Put_Flushcounter_ptr[OSPD_Process_info.my_rank])
             - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
    dst_disp = (size_t)(OSPD_Put_Flushcounter_ptr[proc])
             - (size_t) OSPD_Membase_global[proc] + 1;

    OSPD_Connection_put_active[proc] = 0;
    OSPD_Put_flushack_active++;

    status = DCMF_Put(&OSPD_Generic_put_protocol,
                      &request,
                      OSPD_Nocallback,
                      DCMF_SEQUENTIAL_CONSISTENCY,
                      proc,
                      1,
                      &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                      &OSPD_Memregion_global[proc],
                      src_disp,
                      dst_disp,
                      callback);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Put returned with an error \n");

    OSPDI_Conditional_advance(OSPD_Put_flushack_active > 0);

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPDI_Put_flush_start(int proc)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t ack_callback;
    size_t src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    ack_callback.function = OSPDI_Generic_done;
    ack_callback.clientdata = (void *) &OSPD_Put_flushack_active;

    src_disp = (size_t)(OSPD_Put_Flushcounter_ptr[OSPD_Process_info.my_rank])
             - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
    dst_disp = (size_t)(OSPD_Put_Flushcounter_ptr[proc])
             - (size_t) OSPD_Membase_global[proc] + 1;

    OSPD_Put_flushack_active++;

    status = DCMF_Put(&OSPD_Generic_put_protocol,
                      &request,
                      OSPD_Nocallback,
                      DCMF_SEQUENTIAL_CONSISTENCY,
                      proc,
                      1,
                      &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                      &OSPD_Memregion_global[proc],
                      src_disp,
                      dst_disp,
                      ack_callback);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Put returned with an error \n");

    OSPD_Connection_put_active[proc] = 0;

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Flush(int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (OSPD_Connection_send_active[proc])
    {
        status = OSPDI_Send_flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPDI_Send_flush failed\n");
    }
    else if (OSPD_Connection_put_active[proc])
    {
        status = OSPDI_Put_flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPDI_Put_flush failed \n");
    }

    OSPDI_Conditional_advance(OSPD_Put_flushack_active > 0 || OSPD_Control_flushack_active > 0);

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
