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

/*********** EXTERNAL OBJECTS ***********/

typedef struct {
	pami_geometry_t geometry;
	pami_memregion_t * memregions;
} pamid_gmr_t;

typedef struct {
	pami_geometry_t geometry;
	void * base;
} pamid_window_t;

/*********** EXTERNAL DECLARATIONS ***********/

int PAMID_Initialize(void);
int PAMID_Finalize(void);

size_t PAMID_World_rank(void);
size_t PAMID_World_size(void);

int PAMID_Barrier_world(void);
int PAMID_Broadcast_world(int root, size_t num_bytes, void * buffer);
int PAMID_Allgather_world(size_t num_bytes, void * sbuf, void * rbuf);
int PAMID_Allreduce_world(size_t count, void * sbuf, void * rbuf, pami_type_t type, pami_data_function op);

#endif /* PAMID_H */
