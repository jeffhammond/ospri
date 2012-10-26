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
	int local;
	int remote;
	/* may need extra state for other ops */
} pamid_request_t;

/*********** EXTERNAL DECLARATIONS ***********/

int PAMID_Initialize(void);
int PAMID_Finalize(void);

size_t PAMID_World_rank(void);
size_t PAMID_World_size(void);

int PAMID_Barrier_world(void);
int PAMID_Broadcast_world(int root, size_t num_bytes, void * buffer);
int PAMID_Allgather_world(size_t num_bytes, void * sbuf, void * rbuf);
int PAMID_Allreduce_world(size_t count, void * sbuf, void * rbuf, pami_type_t type, pami_data_function op);

int PAMID_Put_endtoend(size_t bytes, void * local, size_t target, void * remote);
int PAMID_Put_nonblocking(size_t bytes, void * local, size_t target, void * remote, pamid_request_t * request);
int PAMID_Get_endtoend(size_t bytes, void * local, size_t target, void * remote);
int PAMID_Get_nonblocking(size_t bytes, void * local, size_t target, void * remote, pamid_request_t * request);

#endif /* PAMID_H */
