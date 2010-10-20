/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospu.h"
#include "ospd.h"
#include "mpi.h"
#include <assert.h>

/*************************************************
 *                 Constants                     *
 ************************************************/



/*************************************************
 *                  Macros                       *
 *************************************************/



/*************************************************
 *             Data Structures                   *
 *************************************************/

typedef struct
{
   size_t my_rank;
   size_t num_ranks;
} OSPD_Process_info_t;


/*************************************************
 *             Global variables                  *
 ************************************************/



/************************************************* 
 *             Function Prototypes               *
 ************************************************/

