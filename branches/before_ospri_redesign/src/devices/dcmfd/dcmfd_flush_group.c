/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

void OSPDI_Flush_all() 
{
    int status = OSP_SUCCESS;
    int dst,request_count;
    DCMF_Request_t *request;
    DCQuad msginfo;
    DCMF_Callback_t ack_callback;
    volatile int pending_count;
    size_t src_disp, dst_disp;

    OSPU_FUNC_ENTER();

    request_count = (ospd_settings.flushall_pending_limit < OSPD_Process_info.num_ranks) ? 
                      ospd_settings.flushall_pending_limit : OSPD_Process_info.num_ranks;
    status = OSPDI_Malloc((void **) &request, sizeof(DCMF_Request_t) * request_count); 
    OSPU_ERR_POP(status != 0, "OSPDI_Malloc failed in OSPDI_Flush_all\n"); 

    pending_count = 0;
    ack_callback.function = OSPDI_Generic_done;
    ack_callback.clientdata = (void *) &OSPD_Put_flushack_active;

    for (dst = 0; dst < OSPD_Process_info.num_ranks; dst++)
    {
        likely_if (dst != OSPD_Process_info.my_rank)
        {

            if (OSPD_Connection_send_active[dst] > 0)
            {

                OSPD_Control_flushack_active++;

                status = DCMF_Send(&OSPD_Send_flush_protocol,
                                   &request[pending_count],
                                   OSPD_Nocallback,
                                   DCMF_SEQUENTIAL_CONSISTENCY,
                                   dst,
                                   0,
                                   NULL,
                                   &msginfo,
                                   1);
                OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Send returned with an error\n");
                pending_count++;

            }
            else if (OSPD_Connection_put_active[dst] > 0)
            {

                src_disp = (size_t) OSPD_Put_Flushcounter_ptr[OSPD_Process_info.my_rank]
                         - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];
                dst_disp = (size_t) OSPD_Put_Flushcounter_ptr[dst]
                         - (size_t) OSPD_Membase_global[dst] + 1;

                OSPD_Put_flushack_active++;

                status = DCMF_Put(&OSPD_Generic_put_protocol,
                                  &request[pending_count],
                                  OSPD_Nocallback,
                                  DCMF_SEQUENTIAL_CONSISTENCY,
                                  dst,
                                  1,
                                  &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                                  &OSPD_Memregion_global[dst],
                                  src_disp,
                                  dst_disp,
                                  ack_callback);
                OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Put returned with an error\n");
                pending_count++;

            }

            if (pending_count >= ospd_settings.flushall_pending_limit)
            {
                OSPDI_Conditional_advance(OSPD_Control_flushack_active > 0 || OSPD_Put_flushack_active > 0);
                pending_count = 0;
            }

        }
    }
    OSPDI_Conditional_advance(OSPD_Control_flushack_active > 0 || OSPD_Put_flushack_active > 0);

    OSPDI_Memset((void *) OSPD_Connection_send_active, 0, sizeof(uint32_t) * OSPD_Process_info.num_ranks);
    OSPDI_Memset((void *) OSPD_Connection_put_active, 0, sizeof(uint32_t) * OSPD_Process_info.num_ranks);

  fn_exit: 
    OSPDI_Free(request); 
    OSPU_FUNC_EXIT();
    return;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Flush_group(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        OSPDI_Flush_all();
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1, "OSPD_Flush_group not implemented for non-world groups!");
        goto fn_fail;
    }

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
