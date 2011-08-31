/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

/** @file ospu_memory.h */

#ifndef OSPU_MEMORY_H_
#define OSPU_MEMORY_H_

#define OSPU_Malloc(ptr, num) posix_memalign(ptr, OSPC_ALIGNMENT, num)
#define OSPU_Free(ptr) free(ptr)
#define OSPU_Memset(ptr, val, num)  memset(ptr, val, num)
#define OSPU_Memcpy(trg, src, num)  memcpy(trg, src, num)


#endif /* OSPU_MEMORY_H_ */
