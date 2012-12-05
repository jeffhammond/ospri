#ifndef __simple_barrier_h__
#define __simple_barrier_h__

#include <pami.h>

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
void simple_barrier (pami_client_t client, pami_context_t context);



#endif /* __simple_barrier_h__ */

