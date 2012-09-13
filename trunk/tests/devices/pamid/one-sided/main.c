#include <stdio.h>
#include <assert.h>

#include <pami.h>

#include "simple_query.h"

#include "test.h"




int main (int argc, char * argv[])
{
  pami_result_t result;
  pami_client_t client;
  pami_context_t context[2];

  /* Create the PAMI client */
  result = PAMI_ERROR;
  result = PAMI_Client_create ("ClientName", & client, NULL, 0);
  assert (result == PAMI_SUCCESS);

  /*
   * Determine the maximum number of contexts supported by this client for the
   * current job configuration.
   */
  fprintf (stdout, "(%03d) Maximum number of contexts allowed: %d\n", __LINE__, max_contexts (client));
  if (max_contexts (client) < 2)
  {
    fprintf (stdout, "(%03d) This test requires > 1 context; On Blue Gene/Q, set the environment variable 'PAMI_MU_RESOURCES=Minimal' to increase the number of supported contexts for this job configuration.\n", __LINE__);
    return 1;
  } 
 
  /*
   * Create the PAMI communication contexts. The first context will be used
   * directly for better latency; the second context will have async progress
   * enabled.
   */
  result = PAMI_ERROR;
  result = PAMI_Context_createv (client, NULL, 0, context, 2);
  assert (result == PAMI_SUCCESS);


fprintf (stdout, "before test_fn()\n");
  test_fn (argc, argv, client, context);
fprintf (stdout, "after test_fn()\n");



  /* Destroy the context */
  result = PAMI_ERROR;
  result = PAMI_Context_destroyv (context, 2);
  assert (result == PAMI_SUCCESS);

  /* Destroy the client */
  result = PAMI_ERROR;
  PAMI_Client_destroy (client);
fprintf (stderr, "FIXME: status is not set on success! result = %d\n", result);
//  assert (result == PAMI_SUCCESS);

  return 0;
}




