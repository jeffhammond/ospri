
#include <assert.h>
#include <stdio.h>

#include "simple_async_progress.h"








typedef enum
{
  ASYNC_PROGRESS_EVENT_ALL            =    0,
  ASYNC_PROGRESS_EVENT_RECV_INTERRUPT =    1,
  ASYNC_PROGRESS_EVENT_TIMER          =    2,
  ASYNC_PROGRESS_EVENT_EXT            = 1000
} async_progress_event_t;

typedef void (*async_progress_function) (pami_context_t context, void * cookie);

typedef pami_result_t (*async_progress_register_function) (pami_context_t            context,
                                                           async_progress_function   progress_fn,
                                                           async_progress_function   suspend_fn,
                                                           async_progress_function   resume_fn,
                                                           void                    * cookie);

typedef pami_result_t (*async_progress_enable_function) (pami_context_t         context,
                                                         async_progress_event_t event);

typedef pami_result_t (*async_progress_disable_function) (pami_context_t         context,
                                                          async_progress_event_t event);

typedef struct
{
  pami_extension_t extension;

  async_progress_register_function register_fn;
  async_progress_enable_function   enable_fn;
  async_progress_disable_function  disable_fn;

} async_progress_impl_t;

void async_progress_open (pami_client_t      client,
                          async_progress_t * async_progress)
{
  /*
   * Sanity check that the opaque object is large enough to contain the
   * internal async progress 'handle' structure.
   */
  assert (sizeof(async_progress_impl_t) <= sizeof(async_progress_t));

  async_progress_impl_t * async = (async_progress_impl_t *) async_progress;
  pami_extension_t _extension;

  /*
   * Open the async progress extension.
   */
  pami_result_t result = PAMI_ERROR;
  result = PAMI_Extension_open (client, "EXT_async_progress", (pami_extension_t *) & _extension);
  assert (result == PAMI_SUCCESS);

  /*
   * Get the various async progress extension functions.
   */
  async->register_fn = NULL;
  async->register_fn = (async_progress_register_function)
  PAMI_Extension_symbol (_extension, "register");
  assert (async->register_fn != NULL);

  async->enable_fn = NULL;
  async->enable_fn = (async_progress_enable_function)
  PAMI_Extension_symbol (_extension, "enable");
  assert (async->enable_fn != NULL);

  async->disable_fn = NULL;
  async->disable_fn = (async_progress_disable_function)
  PAMI_Extension_symbol (_extension, "disable");
  assert (async->disable_fn != NULL);

  async->extension = _extension;
  return;
}



void async_progress_close (async_progress_t * async_progress)
{
  async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

  /*
   * Close the async progress extension.
   */
  pami_result_t result = PAMI_ERROR;
  result = PAMI_Extension_close (async->extension);
  assert (result == PAMI_SUCCESS);

  return;
}



void async_progress_enable (async_progress_t * async_progress,
                            pami_context_t     context)
{
  pami_result_t result;
  async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

  /*
   * Register the async progress event handlers for this context.
   */
  result = PAMI_ERROR;
  result = async->register_fn (context,
                               NULL,    /* progress function */
                               NULL,    /* suspend function */
                               NULL,    /* resume function */
                               NULL);   /* cookie */
  assert (result == PAMI_SUCCESS);

  /*
   * Enable async progress for this context.
   */
  result = PAMI_ERROR;
  result = async->enable_fn (context, ASYNC_PROGRESS_EVENT_ALL);
  assert (result == PAMI_SUCCESS);


  /*
   * Async progress is enabled when the async progress extension acquires the
   * context lock.
   */
  fprintf (stdout, "(%03d) Waiting for the async progress extension to acquire the context lock.\n", __LINE__);
  do
  {
    result = PAMI_ERROR;
    result = PAMI_Context_trylock (context);
    assert (result != PAMI_ERROR);

    if (result == PAMI_SUCCESS)
      PAMI_Context_unlock (context);
  }
  while (result == PAMI_SUCCESS);
  fprintf (stdout, "(%03d) Async progress enabled for the context.\n", __LINE__);

  return;
}


void async_progress_disable (async_progress_t * async_progress,
                             pami_context_t     context)
{
  pami_result_t result;
  async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

  /*
   * Sanity check that the context lock is held by the async progress extension.
   */
  result = PAMI_ERROR;
  result = PAMI_Context_trylock (context);
  assert (result == PAMI_EAGAIN);


  /*
   * Disable async progress for this context.
   */
  result = PAMI_ERROR;
  result = async->disable_fn (context, ASYNC_PROGRESS_EVENT_ALL);
  assert (result == PAMI_SUCCESS);


  /*
   * Block until the context is no longer under async progress. This occurs
   * when the context lock is released ?
   */
  fprintf (stdout, "(%03d) Waiting to acquire the context lock.\n", __LINE__);
  PAMI_Context_lock (context);
  fprintf (stdout, "(%03d) Acquired the context lock - async progress is now disabled.\n", __LINE__);

  return;
};

