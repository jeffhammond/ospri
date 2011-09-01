/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

/** @file dcmfd_definitions.h */

/*! \addtogroup osp OSPD dcmfd device interface
 * @{
 */

#include <dcmf.h>

/*************************************************
 *                 Constants                     *
 ************************************************/

#define OSPC_ALIGNMENT 16

/*************************************************
 *               Enumerations                    *
 ************************************************/

//typedef enum
//{
//  OSPD_END2END = 0,
//  OSPD_MUTEX_TRYLOCK,
//  OSPD_MUTEX_UNLOCK
//} ospd_flush_t;

/*************************************************
 *             Data Structures                   *
 *************************************************/

typedef struct
{
    int x;
}
ospd_window_properties_t;

typedef struct
{
    ospd_window_properties_t properties;
    int num_ranks;                        /* length of vectors below */
    int * bytes_per_rank;          /* how many bytes does each rank have (may be zero) */
    int * world_ranks;             /* world_ranks[my_rank_in_window_communicator] = my_rank_in_mpi_comm_world */
    DCMF_Memregion_t * memregions; /*   */
}
ospd_window_t;

typedef struct
{
    DCMF_Request_t request;
    volatile int active;
}
ospd_handle_t

/************************************************* 
 *             Function Prototypes               *
 ************************************************/

void OSPDI_Generic_done(void * clientdata, DCMF_Error_t * error);

int OSPDI_Put_initialize();

int OSPDI_Get_initialize();


/*! @} */
