#ifndef __simple_async_progress_h__
#define __simple_async_progress_h__


#include <pami.h>


/**
 * \brief Opaque async progress 'handle'
 *
 * Maintains the state of the async progress extension for a client.
 *
 * Defined for illustrative purposes only.
 */
typedef uintptr_t async_progress_t[8];

/**
 * \brief Open the 'async progress' extension
 *
 * This is a blocking operation.
 *
 * \param [in]  client         The PAMI client to open the extension against
 * \param [out] async_progress An async progress 'handle'; defined for
 *                             illustrative purposes
 */
void async_progress_open (pami_client_t      client,
                          async_progress_t * async_progress);

void async_progress_close (async_progress_t * async_progress);

/**
 * \brief Enable 'async progress' for a communication context
 *
 * This is a blocking operation. This function will not return until
 * asynchronous progress has been enabled for the communication context.
 *
 * Asynchronous progress is enabled without the use of an explicit 'progress'
 * function. This means that the context will be locked, via PAMI_Context_lock(),
 * and the lock will not be released until asynchronous progress is disabled.
 * Consequently, all new communication must be initiated using the
 * thread-safe PAMI_Context_post() function to post 'work' to the context.
 *
 * \note This simple example does not illustrate the use of an explicit
 *       'progress function'.
 *
 * \note This simple example does not illustrate the use of an explicit
 *       'suspend function' nor an explicit 'resume function'.
 *
 * \param [in] async_progress An async progress 'handle'; defined for
 *                            illustrative purposes
 * \param [in] context        The communication context to be advanced
 *                            asynchronously
 */
void async_progress_enable (async_progress_t * async_progress,
                            pami_context_t     context);

void async_progress_disable (async_progress_t * async_progress,
                             pami_context_t     context);



#endif /* __simple_async_progress_h__ */

