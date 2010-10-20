/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

/* FIXME: move this to a more appropriate place once all
 *         this OSPU/OSPD settings crap is sorted out */

#define OSPC_NETWORK_BYPASS 1
/* units of bytes.  Very hardware specific but need to reorganize
 * code before moving into device layer. */
#define OSPC_NETWORK_BYPASS_UPPER_LIMIT_1D 32768
#define OSPC_NETWORK_BYPASS_UPPER_LIMIT_ND 32768

#define OSPC_ARMCI_STRICT_ORDERING 0

OSPU_Settings_t ospu_settings;

int OSPU_Read_parameters(void)
{
    int result = OSP_SUCCESS;
    char* value = NULL;

    OSPU_FUNC_ENTER();

    ospu_settings.network_bypass = OSPC_NETWORK_BYPASS;
    if ((value = getenv("OSP_NETWORK_BYPASS")) != NULL)
    {
        ospu_settings.network_bypass = atoi(value);
    }

    /* The threshold BELOW which we do NIC-bypass.  We do this because
     * some architectures (BG/P) have a DMA that beats CPU-based
     * intranode transfers for large buffers.
     */
    ospu_settings.network_bypass_upper_limit_1d = OSPC_NETWORK_BYPASS_UPPER_LIMIT_1D;
    if ((value = getenv("OSP_NETWORK_BYPASS_UPPER_LIMIT_1D")) != NULL)
    {
        ospu_settings.network_bypass_upper_limit_1d = atoi(value);
    }
    /* For strided, the threshold is much higher. */
    ospu_settings.network_bypass_upper_limit_Nd = OSPC_NETWORK_BYPASS_UPPER_LIMIT_ND;
    if ((value = getenv("OSP_NETWORK_BYPASS_UPPER_LIMIT_ND")) != NULL)
    {
        ospu_settings.network_bypass_upper_limit_Nd = atoi(value);
    }
    /* If bypass is off, just set upper limit to zero so we always
     * use the NIC.  We do not query network_bypass in contiguous ops. */
    if (ospu_settings.network_bypass == 0)
    {
        ospu_settings.network_bypass_upper_limit_1d = 0;
        ospu_settings.network_bypass_upper_limit_Nd = 0;
    }

    ospu_settings.armci_strict_ordering = OSPC_ARMCI_STRICT_ORDERING;
    if ((value = getenv("OSP_ARMCI_STRICT_ORDERING")) != NULL)
    {
        ospu_settings.armci_strict_ordering = atoi(value);
    }
    fn_exit: OSPU_FUNC_EXIT();
    return result;

    fn_fail: goto fn_exit;
}

int OSPU_Print_parameters(void)
{
    int result = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    if ( 0 == OSPD_Process_id(OSP_GROUP_WORLD) )
    {
        OSPU_output_printf("=============== OSPU Parameters ================\n");
        OSPU_output_printf("These are device-independent settings.\n");

        if ( 1==ospu_settings.armci_strict_ordering )
        {
            OSPU_output_printf("ARMCI strict ordering        = %s\n","ON");
        }
        else if ( 0==ospu_settings.armci_strict_ordering)
        {
            OSPU_output_printf("ARMCI strict ordering        = %s\n","OFF");
        }
        else
        {
            OSPU_output_printf("ARMCI strict ordering        = %s\n","WTF");
        }

        if ( 1==ospu_settings.network_bypass )
        {
            OSPU_output_printf("NIC bypass                   = %s\n","ON");
            OSPU_output_printf("NIC bypass upper limit (1D)  = %u\n",ospu_settings.network_bypass_upper_limit_1d);
            OSPU_output_printf("NIC bypass upper limit (ND)  = %u\n",ospu_settings.network_bypass_upper_limit_Nd);

        }
        else if ( 0==ospu_settings.network_bypass)
        {
            OSPU_output_printf("Network bypass               = %s\n","OFF");
        }
        else
        {
            OSPU_output_printf("Network bypass               = %s\n","WTF");
        }
        OSPU_output_printf("===============================================\n\n\n");
        fflush(stdout);
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return result;

  fn_fail:
    goto fn_exit;
}
