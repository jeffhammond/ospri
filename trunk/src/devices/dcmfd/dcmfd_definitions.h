/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

/** @file dcmfd_definitions.h */

/*! \addtogroup osp OSPD dcmfd device interface
 * @{
 */

/*************************************************
 *                 Constants                     *
 ************************************************/

#define OSPC_ALIGNMENT 16

/*************************************************
 *             Data Structures                   *
 *************************************************/

typedef struct
{
    int my_mpi_rank = -1;
    int mpi_world_size = -1;
    int my_dcmf_rank = -1;
    int dcmf_world_size = -1;
    DCMF_Hardware_t hw;
} OSPD_Process_info_t;

typedef struct
{
    int num_ranks;            /* length of vectors below */
    int * bytes_per_rank;     /* how many bytes does each rank have (may be zero) */
    int * world_ranks = NULL; /* world_ranks[my_rank_in_window_communicator] = my_rank_in_mpi_comm_world */
    DCMF_Memregion_t * memregions = NULL;
} ospd_window_t;

typedef struct
{
    DCMF_Request_t request;
    volatile int active = 0;
}
ospd_handle_t

/************************************************* 
 *             Function Prototypes               *
 ************************************************/

void OSPDI_Generic_done(void *, DCMF_Error_t *);

void OSPDI_Request_done(void *, DCMF_Error_t *);

int OSPDI_Put_initialize();

int OSPDI_Get_initialize();


/*! @} */
