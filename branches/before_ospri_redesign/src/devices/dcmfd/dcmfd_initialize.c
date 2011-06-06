/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Configure_t OSPD_Messager_info;
OSPD_Process_info_t OSPD_Process_info;

DCMF_Callback_t OSPD_Nocallback;

int OSPD_Initialize(int thread_level)
{

    int status = OSP_SUCCESS;
    int count = 0;

    OSPU_FUNC_ENTER();

    /* TODO: need a non-DCMF lock here to make this function thread-safe */
    /* TODO: need to set "OSP is alive" global variable */

    count = DCMF_Messager_initialize();
    /* Do not issue this warning if using MPI since in that case we know DCMF
       will be initialized by MPI before OSP (assuming GA->ARMCI->OSP call path). */
    // OMIT THIS WARNING FOR NOW.  WE HAVE BIGGER PROBLEMS AT THE MOMENT.
    //if(!ospd_settings.mpi_active)
    //{
    //    OSPU_WARNING(count == 0,
    //                "DCMF_Messager_initialize has been called more than once.");
    //}

    if ( DCMF_Messager_size() > 1 ) DCMF_Collective_initialize();

    OSPD_Nocallback.function = NULL;
    OSPD_Nocallback.clientdata = NULL;

    status = OSPDI_Read_parameters();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Read_parameters returned with error \n");

    if (ospd_settings.enable_cht)
    {
        if (!ospd_settings.mpi_active)
        {
            /* We can use THREAD_SERIALIZED if we are implementing out own locks
             * ~AND~ MPI is not active */
            OSPD_Messager_info.thread_level = DCMF_THREAD_SERIALIZED;
        }
        else
        {
            /* If MPI is active, DCMF_Critical_section requires DCMF_THREAD_MULTIPLE
             * to work properly */
            OSPD_Messager_info.thread_level = DCMF_THREAD_MULTIPLE;
        }
        OSPD_Messager_info.interrupts = DCMF_INTERRUPTS_OFF;
    }
    else
    {
        switch (thread_level)
        {
        case OSP_THREAD_SINGLE:
            thread_level = DCMF_THREAD_SINGLE;
            break;
        case OSP_THREAD_FUNNELED:
            thread_level = DCMF_THREAD_FUNNELED;
            break;
        case OSP_THREAD_SERIALIZED:
            thread_level = DCMF_THREAD_SERIALIZED;
            break;
        case OSP_THREAD_MULTIPLE:
            thread_level = DCMF_THREAD_MULTIPLE;
            break;
        default:
            OSPU_ERR_POP(OSP_ERROR,
                        "Unsupported thread level provided in OSPD_Initialize \n");
            break;
        }
        OSPD_Messager_info.interrupts = ( ospd_settings.enable_interrupts ? DCMF_INTERRUPTS_ON : DCMF_INTERRUPTS_OFF );
    }

    status = DCMF_Messager_configure(&OSPD_Messager_info, &OSPD_Messager_info);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Messager_configure returned with error \n");

    OSPD_Process_info.my_rank = DCMF_Messager_rank();
    OSPD_Process_info.num_ranks = DCMF_Messager_size();

    /* TODO: initialize node rank/size properly on BGP */
    OSPD_Process_info.my_node = DCMF_Messager_rank();
    OSPD_Process_info.num_nodes = DCMF_Messager_size();

    if (ospd_settings.enable_cht)
    {
        /* Initialize LockBox if it is the locking mechanism used */
        // OSPDI_GLOBAL_LBMUTEX_INITIALIZE();

        /* Create CHT */
        status = pthread_create(&OSPDI_CHT_pthread,
                                NULL,
                                &OSPDI_CHT_advance_lock,
                                NULL);
        OSPU_ERR_POP(status != 0, "pthread_create returned with error \n");
    }

    OSPDI_CRITICAL_ENTER();

    status = OSPDI_Print_parameters();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Print_parameters returned with error \n");

    status = OSPDI_Control_xchange_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Control_xchange_initialize returned with error \n");

    status = OSPDI_Control_flushack_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Control_flushack_initialize returned with error \n");

    status = OSPDI_GlobalBarrier_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_GlobalBarrier_initialize returned with error \n");

    status = OSPDI_GlobalAllreduce_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_GlobalAllreduce_initialize returned with error \n");

    status = OSPDI_GlobalBcast_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_GlobalBcast_initialize returned with error \n");

    status = OSPDI_Put_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Put_initialize returned with error \n");

    status = OSPDI_Get_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Get_initialize returned with error \n");

    status = OSPDI_Putacc_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Putacc_initialize returned with error \n");

    status = OSPDI_Putmod_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Putmod_initialize returned with error \n");

    status = OSPDI_Packed_puts_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Packed_puts_initialize returned with error \n");

    status = OSPDI_Packed_gets_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Packed_gets_initialize returned with error \n");

    status = OSPDI_Packed_putaccs_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Packed_putaccs_initialize returned with error \n");

    status = OSPDI_Rmw_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Rmw_initialize returned with error \n");

    status = OSPDI_Counter_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Counter_initialize returned with error \n");

    status = OSPDI_Send_flush_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Send_flush_initialize returned with error \n");

    status = OSPDI_Put_flush_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "Put flush initialize returned with error \n");

    status = OSPDI_Request_pool_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPDI_Request_pool_initialize failed \n");

    status = OSPDI_Buffer_pool_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Buffer_pool_initialize returned with error \n");

    status = OSPDI_Handle_pool_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPDI_Handle_pool_initialize failed \n");

    /* TODO: Do we need to barrier before this call?
     *       Won't this call fail internally if one process is late? */
    /* Resolution: This function has a barrier inside, before the exchange
     *       happens. So a barrier is not required here */
    status = OSPDI_Memregion_Global_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Memregion_Global_initialize returned with error \n");
  
    /*waiting for everyone*/
    status = OSPDI_GlobalBarrier();
    OSPU_ERR_POP(status != OSP_SUCCESS,
              "OSPDI_GlobalBarrier returned with an error");

  fn_exit: 
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

