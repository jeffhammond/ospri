
#include <assert.h>

#include "simple_barrier.h"

/**
 * \brief Simple barrier event function that decrements the unsigned value pointed to by the cookie
 *
 * This function is used as the completion callback for the simple barrier.
 *
 * \see
 *
 * \param [in] context The PAMI communication context that invoked this function
 * \param [in] cookie  The cookie originally registered for this event
 * \param [in] result  An informational status code
 */
void simple_barrier_decrement (pami_context_t context, void * cookie, pami_result_t result)
{
  unsigned * value = (unsigned *) cookie;
  *value = *value - 1;
}

/**
 * \brief Blocking 'world geometry' barrier
 *
 * This function is provided for illustrative purposes only. One would never
 * include the retrieval of the world geometry and the query of the barrier
 * algorithm in a performance critical code.
 *
 * \param[in] client  The PAMI client; needed to obtain the geometry
 * \param[in] context The PAMI context; used for the barrier communication
 */
void simple_barrier (pami_client_t client, pami_context_t context)
{
  pami_result_t    result;
  pami_geometry_t  world_geometry;
  pami_xfer_t      xfer;
  pami_algorithm_t algorithm;
  pami_metadata_t  metadata;

  /* Retrieve the PAMI 'world' geometry */
  result = PAMI_ERROR;
  result = PAMI_Geometry_world (client, &world_geometry);
  assert (result == PAMI_SUCCESS);

  /* Query the 'always works' barrier algorithm in the geometry */
  result = PAMI_ERROR;
  result = PAMI_Geometry_algorithms_query (world_geometry, PAMI_XFER_BARRIER,
                                           &algorithm, &metadata, 1,
                                           NULL, NULL, 0);
  assert (result == PAMI_SUCCESS);

  /* Set up the barrier */
  volatile unsigned active = 1;
  xfer.cb_done   = simple_barrier_decrement;
  xfer.cookie    = (void *) & active;
  xfer.algorithm = algorithm;

  /* Issue the barrier collective */
  result = PAMI_ERROR;
  result = PAMI_Collective (context, &xfer);
  assert (result == PAMI_SUCCESS);

  /* Advance until the barrier has completed */
  while (active)
  {
    result = PAMI_ERROR;
    result = PAMI_Context_advance (context, 1);
    assert (result == PAMI_SUCCESS);
  }

  return;
}




