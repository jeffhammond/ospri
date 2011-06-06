/* *- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in toplevel directory.
 */

#include "dcmfdimpl.h"

int OSPD_Finalize(void)
{
    int status = OSP_SUCCESS;
    int count = 0;

    OSPU_FUNC_ENTER();

    /* TODO: need to unset "OSP is alive" global variable */

    OSPDI_CRITICAL_ENTER();

    /*waiting for everyone*/
    status = OSPDI_GlobalBarrier();
    OSPU_ERR_POP(status != OSP_SUCCESS, 
              "OSPDI_GlobalBarrier returned with an error");

    /* Freeing request pool */
    OSPDI_Request_pool_finalize();

    /* Freeing handle pool */
    OSPDI_Handle_pool_finalize();

    /* Freeing buffer pool */
    OSPDI_Buffer_pool_finalize();

    /* Freeing memory region pointers and local memroy region*/
    OSPDI_Free(OSPD_Membase_global);
    OSPDI_Free(OSPD_Memregion_global);

    /* Freeing conenction active counters */
    OSPDI_Free((void *) OSPD_Connection_send_active);
    OSPDI_Free((void *) OSPD_Connection_put_active);

    /* Freeing put flush local counters and pointers */
    OSPDI_Free(OSPD_Put_Flushcounter_ptr[OSPD_Process_info.my_rank]);
    OSPDI_Free(OSPD_Put_Flushcounter_ptr);

    if (ospd_settings.enable_cht)
    {
        status = pthread_cancel(OSPDI_CHT_pthread);
    }

    OSPDI_CRITICAL_EXIT();

    /* NOTE: exit critical section before finalize since CS may not work after DCMF is terminated */

    count = DCMF_Messager_finalize();
    /* Do not issue this warning if using MPI since in that case we know DCMF
       will be initialized by MPI before OSP (assuming GA->ARMCI->OSP call path). */
    //if(!ospd_settings.mpi_active)
    //{
    //    OSPU_WARNING(count == 0,
    //                "DCMF_Messager_finalize has been called more than once.");
    //}

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void OSPD_Abort(int error_code, char error_message[])
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(status = OSP_ERROR,
            "User called OSP_ABORT with error code %d, error msg: %s Program terminating abnormally \n",
            error_code,
            error_message);

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

