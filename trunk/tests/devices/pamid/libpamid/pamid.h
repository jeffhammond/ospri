#ifndef PAMID_H
#define PAMID_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

#include "pamiu.h"
#include "internals.h"

/*********** EXTERNAL DECLARATIONS ***********/

int PAMID_Initialize(void);
int PAMID_Finalize(void);

size_t PAMID_World_rank(void);
size_t PAMID_World_size(void);

int PAMID_Barrier_world(void);
int PAMID_Broadcast_world(int root, size_t num_bytes, void * buffer);
int PAMID_Allreduce_world(void)
int PAMID_Allgather_world(void)

#endif /* PAMID_H */
