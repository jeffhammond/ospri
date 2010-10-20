/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

OSPD_Settings_t ospd_settings;

int OSPDI_Read_parameters()
{
    int result = OSP_SUCCESS;
    char* value = NULL;

    OSPU_FUNC_ENTER();

    ospd_settings.alignment = OSPC_ALIGNMENT;

    ospd_settings.enable_cht = OSPC_ENABLE_CHT;
    if ((value = getenv("OSPD_ENABLE_CHT")) != NULL)
    {
        ospd_settings.enable_cht = atoi(value);
    }

    ospd_settings.use_handoff = OSPC_USE_HANDOFF;
    if ((value = getenv("OSPD_USE_HANDOFF")) != NULL)
    {
        ospd_settings.use_handoff = atoi(value);
    }
    if (!ospd_settings.enable_cht)
    {
        ospd_settings.use_handoff = 0;
    }

    ospd_settings.cht_pause_cycles = OSPC_CHT_PAUSE_CYCLES;
    if ((value = getenv("OSPD_CHT_PAUSE_CYCLES")) != NULL)
    {
        ospd_settings.cht_pause_cycles = atoi(value);
    }

    ospd_settings.enable_interrupts = OSPC_ENABLE_INTERRUPTS;
    if ((value = getenv("OSPD_ENABLE_INTERRUPTS")) != NULL)
    {
        ospd_settings.enable_interrupts = atoi(value);
    }

    ospd_settings.mpi_active = OSPC_MPI_ACTIVE;
    if ((value = getenv("OSPD_MPI_ACTIVE")) != NULL)
    {
        ospd_settings.mpi_active = atoi(value);
    }

    ospd_settings.flushall_pending_limit = OSPC_FLUSHALL_PENDING_LIMIT;
    if ((value = getenv("OSPD_FLUSHALL_PENDING_LIMIT")) != NULL)
    {
        ospd_settings.flushall_pending_limit = atoi(value);
    }

    ospd_settings.put_packing_limit = OSPC_PUT_PACKING_LIMIT;
    if ((value = getenv("OSPD_PUT_PACKING_LIMIT")) != NULL)
    {
        ospd_settings.put_packing_limit = atoi(value);
        if (ospd_settings.enable_cht == 0 && ospd_settings.enable_interrupts == 0)
        {
            ospd_settings.put_packing_limit = 0;
        }
    }

    ospd_settings.put_packetsize = OSPC_PUT_PACKETSIZE;
    if ((value = getenv("OSPD_PUT_PACKETSIZE")) != NULL)
    {
        ospd_settings.put_packetsize = atoi(value);
    }
    /* Having packet size less than the twice of packing limit will not make sense */
    if (ospd_settings.put_packetsize < (2*ospd_settings.put_packing_limit
                   +  sizeof(OSPD_Packed_puts_header_t)))
    {
        ospd_settings.put_packetsize = 2*ospd_settings.put_packing_limit
                   +  sizeof(OSPD_Packed_puts_header_t);
    }

    ospd_settings.get_packing_limit = OSPC_GET_PACKING_LIMIT;
    if ((value = getenv("OSPD_GET_PACKING_LIMIT")) != NULL)
    {
        ospd_settings.get_packing_limit = atoi(value);
        if (ospd_settings.enable_cht == 0 && ospd_settings.enable_interrupts == 0)
        {
            ospd_settings.get_packing_limit = 0;
        }
    }

    ospd_settings.get_packetsize = OSPC_GET_PACKETSIZE;
    if ((value = getenv("OSPD_GET_PACKETSIZE")) != NULL)
    {
        ospd_settings.get_packetsize = atoi(value);
    }
    /* Having packet size less than the twice of packing limit will not make sense */
    if (ospd_settings.get_packetsize < (2*ospd_settings.get_packing_limit
                     +  sizeof(OSPD_Packed_gets_header_t)))
    {
        ospd_settings.get_packetsize = 2*ospd_settings.get_packing_limit
                     +  sizeof(OSPD_Packed_gets_header_t);
    }

    ospd_settings.putacc_packing_limit = OSPC_PUTACC_PACKING_LIMIT;
    if ((value = getenv("OSPD_PUTACC_PACKING_LIMIT")) != NULL)
    {
        ospd_settings.putacc_packing_limit = atoi(value);
        if (ospd_settings.enable_cht == 0 && ospd_settings.enable_interrupts == 0)
        {
            ospd_settings.putacc_packing_limit = 0;
        }
    }

    ospd_settings.putacc_packetsize = OSPC_PUTACC_PACKETSIZE;
    if ((value = getenv("OSPD_PUTACC_PACKETSIZE")) != NULL)
    {
        ospd_settings.putacc_packetsize = atoi(value);
    }
    /* Having packet size less than the twice of packing limit will not make sense */
    if (ospd_settings.putacc_packetsize < (2*ospd_settings.putacc_packing_limit
                         + sizeof(OSPD_Packed_putaccs_header_t)))
    {
        ospd_settings.putacc_packetsize = 2*ospd_settings.putacc_packing_limit
                         + sizeof(OSPD_Packed_putaccs_header_t);
    }

    ospd_settings.handlepool_size = OSPC_HANDLE_POOL_SIZE;
    if ((value = getenv("OSPD_HANDLE_POOL_SIZE")) != NULL)
    {
        ospd_settings.handlepool_size = atoi(value);
    }

    ospd_settings.requestpool_size = OSPC_REQUEST_POOL_SIZE;
    if ((value = getenv("OSPD_REQUEST_POOL_SIZE")) != NULL)
    {
        ospd_settings.requestpool_size = atoi(value);
    }

    ospd_settings.put_bufferpool_size = OSPC_PUT_BUFFERPOOL_SIZE;
    if ((value = getenv("OSPD_PUT_BUFFERPOOL_SIZE")) != NULL)
    {
        ospd_settings.put_bufferpool_size = atoi(value);
    }   

    ospd_settings.get_bufferpool_size = OSPC_GET_BUFFERPOOL_SIZE;
    if ((value = getenv("OSPD_GET_BUFFERPOOL_SIZE")) != NULL)
    {
        ospd_settings.get_bufferpool_size = atoi(value);
    } 

    ospd_settings.putacc_bufferpool_size = OSPC_PUTACC_BUFFERPOOL_SIZE;
    if ((value = getenv("OSPD_PUTACC_BUFFERPOOL_SIZE")) != NULL)
    {
        ospd_settings.putacc_bufferpool_size = atoi(value);
    }

    ospd_settings.use_handoff = OSPC_USE_HANDOFF;
    if ((value = getenv("OSPD_USE_HANDOFF")) != NULL)
    {
        ospd_settings.use_handoff = atoi(value);
    }
    if (!ospd_settings.enable_cht)
    {
        ospd_settings.use_handoff = 0;
    }

    fn_exit: OSPU_FUNC_EXIT();
    return result;

    fn_fail: goto fn_exit;
}

int OSPDI_Print_parameters(void)
{
    int result = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    if (OSPD_Process_info.my_rank == 0)
    {
        OSPU_output_printf("=============== OSPD Parameters ================\n");
        OSPU_output_printf("OSP is using the DCMF (Blue Gene/P) device\n");

        OSPU_output_printf("num_ranks                    = %u\n", OSPD_Process_info.num_ranks);
        OSPU_output_printf("num_nodes                    = %u\n", OSPD_Process_info.num_nodes);

        switch (OSPD_Process_info.num_ranks/OSPD_Process_info.num_nodes)
        {
            case 1:
                OSPU_output_printf("node mode                    = %s\n","SMP");
                break;
            case 2:
                OSPU_output_printf("node mode                    = %s\n","DUAL");
                break;
            case 4:
                OSPU_output_printf("node mode                    = %s\n","VN");
                break;
            default:
                OSPU_output_printf("node mode                    = %s\n","WTF");
                break;
        }

        if (ospd_settings.enable_cht)
        {
            OSPU_output_printf("passive progress mode        = %s\n","CHT");
        }
        else if (ospd_settings.enable_interrupts)
        {
            OSPU_output_printf("passive progress mode        = %s\n","DCMF interrupts");
        }
        else
        {
            OSPU_output_printf("passive progress mode        = %s\n","OFF");
        }

        OSPU_output_printf("mpi_active                   = %u\n", ospd_settings.mpi_active);
        OSPU_output_printf("cht_pause_cycles             = %u\n", ospd_settings.cht_pause_cycles);
        OSPU_output_printf("use_handoff                  = %u\n", ospd_settings.use_handoff);

        OSPU_output_printf("get_packing_limit            = %u\n", ospd_settings.get_packing_limit);
        OSPU_output_printf("put_packing_limit            = %u\n", ospd_settings.put_packing_limit);
        OSPU_output_printf("putacc_packing_limit         = %u\n", ospd_settings.putacc_packing_limit);

        OSPU_output_printf("get_packetsize               = %u\n", ospd_settings.get_packetsize);
        OSPU_output_printf("put_packetsize               = %u\n", ospd_settings.put_packetsize);
        OSPU_output_printf("putacc_packetsize            = %u\n", ospd_settings.putacc_packetsize);

        OSPU_output_printf("flushall_pending_limit       = %u\n", ospd_settings.flushall_pending_limit);
        OSPU_output_printf("handlepool_size              = %u\n", ospd_settings.handlepool_size);
        OSPU_output_printf("requestpool_size             = %u\n", ospd_settings.requestpool_size);

        OSPU_output_printf("memory alignment             = %u\n", ospd_settings.alignment);

        fflush(stdout);
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return result;

  fn_fail: 
    goto fn_exit;
}
