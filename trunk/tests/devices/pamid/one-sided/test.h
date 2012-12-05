#ifndef __test_h__
#define __test_h__

#include <pami.h>

/* Each test will implement this function in a separate object file */
void test_fn (int argc, char * argv[],
              pami_client_t client,
              pami_context_t context[]);

#endif /* __test_h__ */

