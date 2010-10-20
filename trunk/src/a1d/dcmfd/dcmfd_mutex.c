/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Mutex_protocol;

int *OSPD_Mutexes_count;
OSPD_Mutex_t *OSPD_Mutexes;

volatile int mutex_request_active = 0;
volatile OSP_bool_t mutex_acquired;

void OSPDI_Mutex_callback(void *clientdata,
                         const DCMF_Control_t *info,
                         size_t peer)
{
    int status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    OSPD_Mutex_pkt_t *mutex_pkt = (OSPD_Mutex_pkt_t *) info;
    OSPD_Mutex_pkt_t response_pkt;
    OSPD_Mutex_request_t *mutex_request;

    if (mutex_pkt->response == -1) 
    {
        /*This is a mutex request packet*/
        if(mutex_pkt->mutex_op == OSPD_MUTEX_LOCK)
        {
            if(OSPD_Mutexes[mutex_pkt->mutex_idx].mutex == -1) 
            {
                OSPD_Mutexes[mutex_pkt->mutex_idx].mutex = peer;
                
                /* other fields do not matter in response packet 
                   as the requester will be waiting for just one
                   mutex response at a time */
                response_pkt.response = OSP_TRUE;

                OSPDI_Memcpy((void *) &cmsg,(void *) &response_pkt, sizeof(OSPD_Mutex_pkt_t));

                status = DCMF_Control(&OSPD_Mutex_protocol,
                                      DCMF_SEQUENTIAL_CONSISTENCY,
                                      peer,
                                      &cmsg);
                OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                       "DCMF_Control failed in OSPDI_Mutex_callback \n"); 
            }
            else
            {
               status = OSPDI_Malloc((void **) &mutex_request, sizeof(OSPD_Mutex_request_t));
               OSPU_ERR_ABORT(status != OSP_SUCCESS,
                       "OSPDI_Malloc failed in OSPDI_Mutex_callback \n");  
               
               if(OSPD_Mutexes[mutex_pkt->mutex_idx].tail == NULL)
               {
                  OSPD_Mutexes[mutex_pkt->mutex_idx].tail = mutex_request;
                  OSPD_Mutexes[mutex_pkt->mutex_idx].head = mutex_request; 
               }
               else
               {
                  OSPD_Mutexes[mutex_pkt->mutex_idx].tail->next = mutex_request; 
                  OSPD_Mutexes[mutex_pkt->mutex_idx].tail = mutex_request;
               }
            }
        } 
        else if(mutex_pkt->mutex_op == OSPD_MUTEX_TRYLOCK)
        {
            /* other fields does not matter in response packet
               as the requestor will be waiting for just one
               mutex response at a time */
            if(OSPD_Mutexes[mutex_pkt->mutex_idx].mutex == -1)
            {
                OSPD_Mutexes[mutex_pkt->mutex_idx].mutex = peer;
                response_pkt.response = OSP_TRUE;
            }
            else
            {
                response_pkt.response = OSP_FALSE;
            }

            OSPDI_Memcpy((void *) &cmsg,(void *) &response_pkt, sizeof(OSPD_Mutex_pkt_t));

            status = DCMF_Control(&OSPD_Mutex_protocol,
                                  DCMF_SEQUENTIAL_CONSISTENCY,
                                  peer,
                                  &cmsg);
            OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                   "DCMF_Control failed in OSPDI_Mutex_callback \n");              
        }
        else if(mutex_pkt->mutex_op == OSPD_MUTEX_UNLOCK) 
        {
            if(OSPD_Mutexes[mutex_pkt->mutex_idx].mutex != peer)
            {
                OSPU_ERR_ABORT(status = OSP_ERROR,
                   "Invalid unlock request received \n");     
            }
 
            if(OSPD_Mutexes[mutex_pkt->mutex_idx].head != NULL)
            {
                /*retrieve the next request from the queue*/
                mutex_request = OSPD_Mutexes[mutex_pkt->mutex_idx].head;
                OSPD_Mutexes[mutex_pkt->mutex_idx].head = OSPD_Mutexes[mutex_pkt->mutex_idx].head->next;

                OSPD_Mutexes[mutex_pkt->mutex_idx].mutex = mutex_request->rank; 

                response_pkt.response = OSP_TRUE;

                OSPDI_Memcpy((void *) &cmsg,(void *) &response_pkt, sizeof(OSPD_Mutex_pkt_t));

                status = DCMF_Control(&OSPD_Mutex_protocol,
                                      DCMF_SEQUENTIAL_CONSISTENCY,
                                      mutex_request->rank,
                                      &cmsg);
                OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                      "DCMF_Control failed in OSPDI_Mutex_callback \n");               

                OSPDI_Free(mutex_request); 
            }
            else
            { 
                OSPD_Mutexes[mutex_pkt->mutex_idx].mutex = -1;  
            }
        }
        else
        {
            OSPU_ERR_ABORT(status = OSP_ERROR,
                   "Invalid mutex request received \n");

        }
    }
    else
    {
        /*this is a mutex reponse packet*/
        mutex_acquired = mutex_pkt->response;        

        mutex_request_active--; 
    }
}

int OSPDI_Mutex_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Control_Configuration_t conf;

    OSPU_FUNC_ENTER();

    /* Protocol used for mutex operations */
    conf.protocol = DCMF_DEFAULT_CONTROL_PROTOCOL;
    conf.network = DCMF_DEFAULT_NETWORK;
    conf.cb_recv = OSPDI_Mutex_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Control_register(&OSPD_Mutex_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "Mutexes protocol registration returned with error %d \n",
                status);

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Create_mutexes(OSP_group_t* group, int mutex_count, int *mutex_count_ar)
{
    int index, rank, status = OSP_SUCCESS;
    DCMF_Control_t cmsg;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (group != OSP_GROUP_WORLD),
                "Mutexes are not implemented for non-world groups!");

    status = OSPDI_Malloc((void **) &OSPD_Mutexes, sizeof(OSPD_Mutex_t)*mutex_count);
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc returned error in OSPD_Create_counter\n");

    for(index=0; index<mutex_count; index++)
    {
       OSPD_Mutexes[index].mutex = -1;  
    }

    status = OSPDI_Malloc((void **) &OSPD_Mutexes_count, sizeof(int)*OSPD_Process_info.num_ranks);
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc returned error in OSPD_Create_counter\n");   

    OSPD_Mutexes_count[OSPD_Process_info.my_rank] = mutex_count;

    /*Exchanging mutex count information among processes in the group*/
    OSPD_Control_xchange_info.xchange_ptr = (void *) OSPD_Mutexes_count;
    OSPD_Control_xchange_info.xchange_size = sizeof(int);
    OSPD_Control_xchange_info.rcv_active += OSPD_Process_info.num_ranks - 1;

    OSPDI_GlobalBarrier();

    OSPDI_Memcpy((void *) &cmsg,
                (void *) &OSPD_Mutexes_count[OSPD_Process_info.my_rank],
                sizeof(int));

    for (rank = 0; rank < OSPD_Process_info.num_ranks; rank++)
    {
        likely_if (rank != OSPD_Process_info.my_rank)
        {
            DCMF_Control(&OSPD_Control_xchange_info.protocol,
                         DCMF_SEQUENTIAL_CONSISTENCY,
                         rank,
                         &cmsg);
        }
    }

    OSPDI_Conditional_advance(OSPD_Control_xchange_info.rcv_active > 0); 

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Destroy_mutexes(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (group != OSP_GROUP_WORLD),
                "Mutexes are not implemented for non-world groups!");

    OSPDI_GlobalBarrier();

    OSPDI_Free(OSPD_Mutexes);

    OSPDI_Free(OSPD_Mutexes_count);

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Lock_mutex(OSP_group_t* group, int mutex, int proc)
{
    int status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    OSPD_Mutex_pkt_t packet;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (mutex > OSPD_Mutexes_count[proc]),
                "Non-existent mutex being requested!");

    mutex_request_active++;

    packet.mutex_idx = mutex;
    packet.mutex_op = OSPD_MUTEX_LOCK;
    packet.response = -1; 

    OSPDI_Memcpy((void *) &cmsg,(void *) &packet, sizeof(OSPD_Mutex_pkt_t));      

    status = DCMF_Control(&OSPD_Mutex_protocol,
                          DCMF_SEQUENTIAL_CONSISTENCY,
                          proc,
                          &cmsg);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Control failed in OSPD_Lock_mutex \n");

    OSPDI_Conditional_advance(mutex_request_active > 0);

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Trylock_mutex(OSP_group_t* group, int mutex, int proc, OSP_bool_t *acquired)
{
    int status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    OSPD_Mutex_pkt_t packet;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (mutex > OSPD_Mutexes_count[proc]),
                "Non-existent mutex being requested!");

    mutex_request_active++;

    packet.mutex_idx = mutex;
    packet.mutex_op = OSPD_MUTEX_TRYLOCK;
    packet.response = -1;

    OSPDI_Memcpy((void *) &cmsg,(void *) &packet, sizeof(OSPD_Mutex_pkt_t));

    status = DCMF_Control(&OSPD_Mutex_protocol,
                          DCMF_SEQUENTIAL_CONSISTENCY,
                          proc,
                          &cmsg);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Control failed in OSPD_Lock_mutex \n");

    OSPDI_Conditional_advance(mutex_request_active > 0);

    *acquired = mutex_acquired;            

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSPD_Unlock_mutex(OSP_group_t* group, int mutex, int proc)
{
    int status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    OSPD_Mutex_pkt_t packet;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ERR_POP(status = (mutex > OSPD_Mutexes_count[proc]),
                "Non-existent mutex being requested!");

    packet.mutex_idx = mutex;
    packet.mutex_op = OSPD_MUTEX_UNLOCK;
    packet.response = -1;

    OSPDI_Memcpy((void *) &cmsg,(void *) &packet, sizeof(OSPD_Mutex_pkt_t));

    status = DCMF_Control(&OSPD_Mutex_protocol,
                          DCMF_SEQUENTIAL_CONSISTENCY,
                          proc,
                          &cmsg);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Control failed in OSPD_Lock_mutex \n");

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
