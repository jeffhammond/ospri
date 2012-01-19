/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#if !defined DCMFD_DEFINITIONS_H_INCLUDED
#define DCMFD_DEFINITIONS_H_INCLUDED

#include <dcmf.h>

/** @file dcmfd_definitions.h */

/*! \addtogroup osp OSPD dcmfd device interface
 * @{
 */

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

typedef enum
{
    OSPD_PUT_HANDLE,
    OSPD_PUT_BULK,
    OSPD_PUT_END2END,
}
ospd_put_hint_t;

/*************************************************
 *             Data Structures                   *
 *************************************************/

typedef struct
{
    /* can get both from DCMF_Memregion_query */
    void * base_ptr;
    /* unnecessary for correct programs */
    size_t size;
    /* not needed if one memregion per node */
    DCMF_Memregion_t memregions;
}
ospd_local_memregion_t;

typedef struct
{
    int bulk_remote_completion_available;
    int num_ranks;                             /* length of vectors below */
    int * world_ranks;                         /* world_ranks[my_rank_in_window_communicator] = my_rank_in_mpi_comm_world */
    ospd_local_memregion_t * local_memregions; /*   */
}
ospd_global_memregion_t;

typedef struct
{
    DCMF_Request_t request;
    volatile int active;
}
ospd_handle_t;

/************************************************* 
 *             Function Prototypes               *
 ************************************************/

void OSPDI_Generic_done(void * clientdata, DCMF_Error_t * error);

int OSPDI_Put_initialize();

int OSPDI_Get_initialize();

/*! @} */

#endif
