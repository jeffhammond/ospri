/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfd_all.h"

MPI_Comm OSP_COMM_WORLD;

int my_mpi_rank = -1;
int mpi_world_size = -1;
int my_dcmf_rank = -1;
int dcmf_world_size = -1;

int OSPD_Initialize(int thread_level)
{
    osp_result_t status = OSP_SUCCESS;

    DCMF_Configure_t OSPD_Messager_info;

    OSPU_FUNC_ENTER();

    /* Check MPI status */
    {
      int mpi_is_init, mpi_is_fin, mpi_provided;
      MPI_Initialized(&mpi_is_init);
      MPI_Finalized(&mpi_is_fin);
      OSPU_ERR_POP( !mpi_is_init || mpi_is_fin , "MPI must be initialized and not finalized!");

      MPI_Query_thread(&mpi_provided);
      OSPU_ERR_POP( mpi_provided != MPI_THREAD_MULTIPLE , "MPI_THREAD_MULTIPLE is required for now.");
    }

    MPI_Comm_dup(MPI_COMM_WORLD, &OSP_COMM_WORLD);

    /* TODO: does MPI_Init do this??? */
    if ( DCMF_Messager_size() > 1 ) DCMF_Collective_initialize();

    /* for now, keep it simple */
    OSPD_Messager_info.thread_level = DCMF_THREAD_MULTIPLE;
    OSPD_Messager_info.interrupts = DCMF_INTERRUPTS_ON;

    status = DCMF_Messager_configure(&OSPD_Messager_info, &OSPD_Messager_info);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Messager_configure returned with error \n");

    /* cache these even though they should be fast */
    MPI_Comm_rank(OSP_COMM_WORLD,&my_mpi_rank);
    MPI_Comm_size(OSP_COMM_WORLD,&mpi_world_size);
    my_dcmf_rank    = DCMF_Messager_rank();
    dcmf_world_size = DCMF_Messager_size();
    OSPU_WARNING( my_dcmf_rank != my_mpi_rank , "WARNING: DCMF and MPI ranks not the same!");
    OSPU_WARNING( dcmf_world_size != mpi_world_size , "WARNING: DCMF and MPI world sizes are not the same!");

    /* initialize various communication protocols */

    /* this is the null callback that is use for many protocols */
    OSPD_Nocallback.function = NULL;
    OSPD_Nocallback.clientdata = NULL;

    status = OSPD_Put_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Put_initialize failed");

    status = OSPD_Get_initialize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Get_initialize failed");

    /**********************************************/

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

