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

void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

/*********** DECLARATIONS ***********/
int PAMID_Progess_setup(int open, pami_context_t context);
int PAMID_Progess_teardown(int close, pami_context_t context);

int PAMID_Barrier_setup(pami_geometry_t geometry, pamid_barrier_state_t * barrier);
int PAMID_Barrier_teardown(pamid_barrier_state_t * barrier);
int PAMID_Barrier_doit(pamid_barrier_state_t * barrier);

#endif /* PAMID_H */
