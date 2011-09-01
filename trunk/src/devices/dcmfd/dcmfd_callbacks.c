/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfd_all.h"

extern DCMF_Callback_t OSPD_Nocallback;

void OSPDI_Generic_done(void * clientdata, DCMF_Error_t * error)
{
    --(*((uint32_t *) clientdata));
}

