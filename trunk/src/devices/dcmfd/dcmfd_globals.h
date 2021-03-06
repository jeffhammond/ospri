/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#if !defined DCMFD_GLOBALS_H_INCLUDED
#define DCMFD_GLOBALS_H_INCLUDED

#include <mpi.h>

/** @file dcmfd_globals.h */

/*! \addtogroup osp OSPD dcmfd device interface
 * @{
 */

/*************************************************
 *             Global variables                  *
 ************************************************/

/* it seems silly to put this into a struct */
extern int my_mpi_rank;
extern int mpi_world_size;
extern int my_dcmf_rank;
extern int dcmf_world_size;
extern MPI_Comm OSP_COMM_WORLD;

extern DCMF_Callback_t OSPD_Nocallback;

extern DCMF_Protocol_t OSPD_Put_protocol;

extern DCMF_Protocol_t OSPD_Get_protocol;

/*! @} */

#endif
