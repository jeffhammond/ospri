#include "pamid.h"

static pami_extension_t async_prog_ext;

typedef void (*pamix_async_function) (pami_context_t context, void * cookie);

typedef enum { PAMI_ASYNC_ALL = 0 } pamix_async_t;

static pami_result_t (*async_prog_register)(pami_context_t context,
		pamix_async_function progress_fn,
		pamix_async_function suspend_fn,
		pamix_async_function resume_fn,
		void *cookie);
static pami_result_t (*async_prog_enable)(pami_context_t context, pamix_async_t event_type);
static pami_result_t (*async_prog_disable)(pami_context_t context, pamix_async_t event_type);

int PAMID_Progess_setup(void)
{
	pami_result_t rc = PAMI_ERROR;

	rc = PAMI_Extension_open(NULL, "EXT_async_progress", &async_prog_ext);
	PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Extension_open - EXT_async_progress");

	async_prog_register = PAMI_Extension_symbol(async_prog_ext, "register");
	PAMID_ASSERT(async_prog_register!=NULL,"PAMI_Extension_symbol - async_prog_register");

	async_prog_enable = PAMI_Extension_symbol(async_prog_ext, "enable");
	PAMID_ASSERT(async_prog_enable!=NULL,"PAMI_Extension_symbol - async_prog_enable");

	async_prog_disable = PAMI_Extension_symbol(async_prog_ext, "disable");
	PAMID_ASSERT(async_prog_disable!=NULL,"PAMI_Extension_symbol - async_prog_disable");

	return PAMI_SUCCESS;
}

int PAMID_Progess_teardown(void)
{
  pami_result_t rc = PAMI_ERROR;
  rc = PAMI_Extension_close(async_prog_ext);
  PAMID_ASSERT(rc==PAMI_SUCCESS,"PAMI_Extension_close - EXT_async_progress");

  return;
}

