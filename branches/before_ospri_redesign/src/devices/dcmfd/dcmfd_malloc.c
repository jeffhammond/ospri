/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Memregion_t *OSPD_Memregion_global;
void **OSPD_Membase_global;

int OSPDI_Memregion_Global_xchange()
{

    int status = OSP_SUCCESS;
    DCMF_Control_t info;
    int rank;

    OSPU_FUNC_ENTER();

    OSPD_Control_xchange_info.xchange_ptr = (void *) OSPD_Memregion_global;
    OSPD_Control_xchange_info.xchange_size = sizeof(DCMF_Memregion_t);
    OSPD_Control_xchange_info.rcv_active += OSPD_Process_info.num_ranks - 1;

    OSPDI_GlobalBarrier();

    OSPDI_Memcpy((void *) &info,
                (void *) &OSPD_Memregion_global[OSPD_Process_info.my_rank],
                sizeof(DCMF_Memregion_t));
    for (rank = 0; rank < OSPD_Process_info.num_ranks; rank++)
    {
        likely_if (rank != OSPD_Process_info.my_rank)
        {
            status = DCMF_Control(&OSPD_Control_xchange_info.protocol,
                                  DCMF_SEQUENTIAL_CONSISTENCY,
                                  rank,
                                  &info);
            OSPU_ERR_POP(status != DCMF_SUCCESS,
                        "DCMF_Control failed in OSPDI_Memregion_Global_xchange\n");
        }
    }
    OSPDI_Conditional_advance(OSPD_Control_xchange_info.rcv_active > 0);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int OSPDI_Memregion_Global_initialize()
{

    int status = OSP_SUCCESS;
    unsigned int out, i;

    OSPU_FUNC_ENTER();

    status = OSPDI_Malloc((void **) &OSPD_Memregion_global,
                                 sizeof(DCMF_Memregion_t) * OSPD_Process_info.num_ranks);
    OSPU_ERR_POP(status != 0, "OSPDI_Malloc failed \n");

    status  = DCMF_Memregion_create(&OSPD_Memregion_global[OSPD_Process_info.my_rank],
                                    &out,
                                    (size_t) - 1,
                                    NULL,
                                    0);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Memregion_create failed \n");

    status = OSPDI_Memregion_Global_xchange();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPDI_Memregion_Global_xchange failed \n");

    status = OSPDI_Malloc((void **) &OSPD_Membase_global, 
                                 sizeof(void *) * OSPD_Process_info.num_ranks);
    OSPU_ERR_POP(status != 0, "OSPDI_Malloc failed \n");

    for (i = 0; i < OSPD_Process_info.num_ranks; i++)
    {
        status = DCMF_Memregion_query(&OSPD_Memregion_global[i],
                                      &out,
                                      (void **) &OSPD_Membase_global[i]);
        OSPU_ERR_POP(status != DCMF_SUCCESS, "Memregion query failed \n");
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPDI_Memaddress_xchange(void **ptr)
{

    int status = OSP_SUCCESS;
    DCMF_Control_t cmsg;
    int rank, bytes;

    OSPU_FUNC_ENTER();

    OSPD_Control_xchange_info.xchange_ptr = (void *) ptr;
    OSPD_Control_xchange_info.xchange_size = sizeof(void *);
    OSPD_Control_xchange_info.rcv_active += OSPD_Process_info.num_ranks - 1;

    OSPDI_GlobalBarrier();

    OSPDI_Memcpy((void *) &cmsg,
                (void *) &ptr[OSPD_Process_info.my_rank],
                sizeof(void *));

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

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int OSPD_Exchange_segments(OSP_group_t* group, void **ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if(group != OSP_GROUP_WORLD && group != NULL)
    {
       OSPU_ERR_POP(OSP_ERROR, "Groups are currently not supported in OSP\n");
    }

    status = OSPDI_Memaddress_xchange(ptr);
    OSPU_ERR_POP(status, "OSPDI_Memaddress_xchange returned with error \n");

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_Alloc_segment(void** ptr, int bytes)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    status = OSPDI_Malloc(ptr, bytes);
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc returned error in OSPD_Alloc_segment\n");

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

