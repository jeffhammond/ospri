#include "pamid.h"

/********************************************************************/

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

/********************************************************************/

typedef enum
{
	ASYNC_PROGRESS_EVENT_ALL            =    0,
	ASYNC_PROGRESS_EVENT_RECV_INTERRUPT =    1,
	ASYNC_PROGRESS_EVENT_TIMER          =    2,
	ASYNC_PROGRESS_EVENT_EXT            = 1000
} async_progress_event_t;

typedef void (*async_progress_function) (pami_context_t context, void * cookie);

typedef pami_result_t (*async_progress_register_function) (pami_context_t context,
		async_progress_function progress_fn,
		async_progress_function suspend_fn,
		async_progress_function resume_fn,
		void * cookie);

typedef pami_result_t (*async_progress_enable_function) (pami_context_t context, async_progress_event_t event);

typedef pami_result_t (*async_progress_disable_function) (pami_context_t context, async_progress_event_t event);

typedef struct
{
	pami_extension_t extension;
	async_progress_register_function register_fn;
	async_progress_enable_function   enable_fn;
	async_progress_disable_function  disable_fn;
} async_progress_impl_t;

void async_progress_open(async_progress_t * async_progress)
{
	pami_result_t rc = PAMI_ERROR;
	async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

	/* Sanity check that the opaque object is large enough to contain the internal async progress 'handle' structure. */
	PAMID_ASSERT(sizeof(async_progress_impl_t) <= sizeof(async_progress_t),"async_progress_open");

	/* Open the async progress extension. */
	rc = PAMI_Extension_open(PAMID_INTERNAL_STATE.client, "EXT_async_progress", (pami_extension_t *) &async->extension);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Extension_open - EXT_async_progress");

	/* Get the various async progress extension functions.  */
	async->register_fn = NULL;
	async->register_fn = (async_progress_register_function) PAMI_Extension_symbol(async_progress_extension, "register");
	PAMID_ASSERT(async_prog_register!=NULL,"PAMI_Extension_symbol - async_prog_register");

	async->enable_fn = NULL;
	async->enable_fn = (async_progress_enable_function) PAMI_Extension_symbol(async_progress_extension, "enable");
	PAMID_ASSERT(async_prog_enable!=NULL,"PAMI_Extension_symbol - async_prog_enable");

	async->disable_fn = NULL;
	async->disable_fn = (async_progress_disable_function) PAMI_Extension_symbol(async_progress_extension, "disable");
	PAMID_ASSERT(async_prog_disable!=NULL,"PAMI_Extension_symbol - async_prog_disable");

	async->extension = async_progress_extension;

	return;
}

void async_progress_close(async_progress_t * async_progress)
{
	pami_result_t rc = PAMI_ERROR;
	async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

	/* Close the async progress extension. */
	rc = PAMI_Extension_close (async->extension);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Extension_close - EXT_async_progress");

	return;
}

void async_progress_enable(async_progress_t * async_progress, pami_context_t context)
{
	pami_result_t rc = PAMI_ERROR;
	async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

	/* Register the async progress event handlers for this context. */
	rc = PAMI_ERROR;
	rc = async->register_fn(context,
			NULL,    /* progress function */
			NULL,    /* suspend function */
			NULL,    /* resume function */
			NULL);   /* cookie */
	PAMID_ASSERT(rc==PAMI_SUCCESS,"async->register_fn");

	/* Enable async progress for this context. */
	rc = PAMI_ERROR;
	rc = async->enable_fn(context, ASYNC_PROGRESS_EVENT_ALL);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"async->enable_fn");

	/* Async progress is enabled when the async progress extension acquires the context lock. */
	fprintf (stdout, "(%03d) Waiting for the async progress extension to acquire the context lock.\n", __LINE__);
	do
	{
		rc = PAMI_ERROR;
		rc = PAMI_Context_trylock (context);
		PAMID_ASSERT(rc!=PAMI_ERROR,"PAMI_Context_trylock");

		if (rc == PAMI_SUCCESS)
			PAMI_Context_unlock (context);
	}
	while (rc == PAMI_SUCCESS);

	return;
}

void async_progress_disable(async_progress_t * async_progress, pami_context_t context)
{
	pami_result_t rc = PAMI_ERROR;
	async_progress_impl_t * async = (async_progress_impl_t *) async_progress;

	/* Sanity check that the context lock is held by the async progress extension. */
	rc = PAMI_ERROR;
	rc = PAMI_Context_trylock(context);
	PAMID_ASSERT(rc==PAMI_EAGAIN,"PAMI_Context_trylock");

	/* Disable async progress for this context. */
	rc = PAMI_ERROR;
	rc = async->disable_fn(context, ASYNC_PROGRESS_EVENT_ALL);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"async->disable_fn");

	/* Block until the context is no longer under async progress. This occurs when the context lock is released? */
	rc = PAMI_Context_lock(context);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Context_trylock");

	return;
}

int PAMID_Progess_setup(pami_context_t context)
{
	//pami_result_t rc = PAMI_ERROR;


	return PAMI_SUCCESS;
}

int PAMID_Progess_teardown(void)
{

	return PAMI_SUCCESS;
}

