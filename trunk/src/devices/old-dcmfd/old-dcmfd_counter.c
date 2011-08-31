/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Counter_create_protocol;
volatile int counter_create_active;
long* counter_create_response;

DCMF_Protocol_t OSPD_Counter_protocol;
volatile int counter_incr_active;
long counter_incr_response;

void OSPDI_Counter_create_callback(void *clientdata,
                                  const DCMF_Control_t *info,
                                  size_t peer)
{
    OSPDI_Memcpy((void *) &counter_create_response, (void *) info, sizeof(void *));
    counter_create_active--;
}

void OSPDI_Counter_callback(void *clientdata,
                           const DCMF_Control_t *info,
                           size_t peer)
{
    int status = OSP_SUCCESS;
    OSPD_Counter_pkt_t *counter_pkt = (OSPD_Counter_pkt_t *) info;

    if (counter_pkt->value_ptr == NULL)
    {
        /*This is a response packet*/
        counter_incr_response = counter_pkt->value;
        counter_incr_active--;
    }
    else
    {
        OSPD_Counter_pkt_t response_pkt;
        DCMF_Control_t cmsg;
        long original;

        original = *(counter_pkt->value_ptr);
        *(counter_pkt->value_ptr) += response_pkt.value;

        response_pkt.value_ptr = NULL;
        response_pkt.value = original;

        OSPDI_Memcpy(&cmsg, &response_pkt, sizeof(OSPD_Counter_pkt_t));

        status = DCMF_Control(&OSPD_Counter_protocol,
                              DCMF_SEQUENTIAL_CONSISTENCY,
                              peer,
                              &cmsg);
        OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                      "DCMF_Control failed in OSPDI_Counter_callback\n");
    }
}

int OSPDI_Counter_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Control_Configuration_t conf;

    OSPU_FUNC_ENTER();

    /* Protocol used to create counters */
    conf.protocol = DCMF_DEFAULT_CONTROL_PROTOCOL;
    conf.network = DCMF_DEFAULT_NETWORK;
    conf.cb_recv = OSPDI_Counter_create_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Control_register(&OSPD_Counter_create_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Control_register returned with error %d \n",
                status);

    /* Protocol used for counter operations */
    conf.protocol = DCMF_DEFAULT_CONTROL_PROTOCOL;
    conf.network = DCMF_DEFAULT_NETWORK;
    conf.cb_recv = OSPDI_Counter_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Control_register(&OSPD_Counter_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "Counter protocol registartion returned with error %d \n",
                status);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Create_counter(OSP_group_t* group, OSP_counter_t *counter_ptr)
{
    int index, status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    OSPD_Counter_t *ospd_counter;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (group != OSP_GROUP_WORLD),
                "Counters are not implemented for non-world groups!");

    status = OSPDI_Malloc((void **) &ospd_counter, sizeof(OSPD_Counter_t));
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc returned error in OSPD_Create_counter\n");

    /* TODO: We have to find a way to select the location of counters dynamically. Will 
     distributing counters across processes in a round-robin fashion be a good way? */
    ospd_counter->rank = OSPD_Process_info.num_ranks - 1;

    unlikely_if (ospd_counter->rank == OSPD_Process_info.my_rank)
    {

        ospd_counter->value_ptr = NULL;

        for (index = 0; index < OSPD_Process_info.num_ranks; index++)
        {
            if (index != OSPD_Process_info.my_rank)
            {
                status = DCMF_Control(&OSPD_Counter_create_protocol,
                                      DCMF_SEQUENTIAL_CONSISTENCY,
                                      index,
                                      &cmsg);
                OSPU_ERR_POP(status != DCMF_SUCCESS,
                            "DCMF_Control failed in OSPD_Alloc_counter\n");
            }
        }

    }
    else
    {
        counter_create_active++;

        OSPDI_Conditional_advance(counter_create_active > 0);

        ospd_counter->value_ptr = counter_create_response;
    }

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Destroy_counter(OSP_group_t* group, OSP_counter_t *counter_ptr)
{
    int index, status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    OSPD_Counter_t *ospd_counter;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (group != OSP_GROUP_WORLD),
                "Counters are not implemented for non-world groups!");

    OSPDI_GlobalBarrier();

    ospd_counter = *counter_ptr;

    OSPDI_Free(ospd_counter);

    *counter_ptr = NULL;

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Incr_counter(OSP_counter_t counter, long increment, long* original)
{
    int status = OSP_SUCCESS;
    OSPD_Counter_t *ospd_counter;
    OSPD_Counter_pkt_t counter_pkt;
    DCMF_Control_t cmsg;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_counter = (OSPD_Counter_t *) counter;

    if (ospd_counter->rank == OSPD_Process_info.my_rank)
    {
        *original = ospd_counter->value;
        ospd_counter->value = ospd_counter->value + increment;
    }
    else
    {

        counter_pkt.value_ptr = ospd_counter->value_ptr;
        counter_pkt.value = increment;

        OSPDI_Memcpy(&cmsg, &counter_pkt, sizeof(OSPD_Counter_pkt_t));

        counter_incr_active = 1;

        status = DCMF_Control(&OSPD_Counter_protocol,
                              DCMF_SEQUENTIAL_CONSISTENCY,
                              ospd_counter->rank,
                              &cmsg);
        OSPU_ERR_POP(status != DCMF_SUCCESS,
                    "DCMF_Control failed in OSPD_Incr_counter\n");

        OSPDI_Conditional_advance(counter_incr_active > 0);

        *original = counter_incr_response;

    }

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

