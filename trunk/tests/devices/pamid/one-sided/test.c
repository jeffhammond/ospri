
#include <string.h>
#include <stdio.h>

#include <pami.h>

#include "simple_barrier.h"
#include "simple_async_progress.h"
#include "accumulate_data_functions.h"

#define MEMREGION_EXCHANGE_DISPATCH_ID 20
#define ACCUMULATE_TEST_DISPATCH_ID    21

#define ASYNC_PROGRESS

void decrement (pami_context_t context, void * cookie, pami_result_t result)
{
  unsigned * value_ptr = (unsigned *) cookie;
  (*value_ptr)--;
}

typedef enum
{
  ACCUMULATE_TEST_SCALAR_SUM = 0,
  ACCUMULATE_TEST_VECTOR_SUM,
  ACCUMULATE_TEST_SCALAR_SUBTRACT,
  ACCUMULATE_TEST_VECTOR_SUBTRACT,
  ACCUMULATE_TEST_VECTOR_MAX_SUM,
  ACCUMULATE_TEST_VECTOR_MIN_SUM,
  ACCUMULATE_TEST_COUNT
} accumulate_test_t;

char * accumulate_test_name[] = {
  "ACCUMULATE_TEST_SCALAR_SUM",
  "ACCUMULATE_TEST_VECTOR_SUM",
  "ACCUMULATE_TEST_SCALAR_SUBTRACT",
  "ACCUMULATE_TEST_VECTOR_SUBTRACT",
  "ACCUMULATE_TEST_VECTOR_MAX_SUM",
  "ACCUMULATE_TEST_VECTOR_MIN_SUM",
  "ACCUMULATE_TEST_COUNT"
};

typedef struct memregion_information
{
  struct iovec     data;
  pami_memregion_t memregion[2];
  unsigned         active;
} memregion_information_t;

void exchange_memregion_recv_cb (pami_context_t    context,
                                 void            * cookie,
                                 const void      * header_addr,
                                 size_t            header_size,
                                 const void      * pipe_addr,
                                 size_t            data_size,
                                 pami_endpoint_t   origin,
                                 pami_recv_t     * recv)
{
  memregion_information_t * info = (memregion_information_t *) cookie;

  pami_task_t origin_task;
  size_t origin_offset;
  PAMI_Endpoint_query (origin, & origin_task, & origin_offset);

  pami_memregion_t * memregion = (pami_memregion_t *) pipe_addr;

  memcpy ((void *) & info[origin_task].memregion[0], (void *) & memregion[0], sizeof(pami_memregion_t));
  memcpy ((void *) & info[origin_task].memregion[1], (void *) & memregion[1], sizeof(pami_memregion_t));


  info[origin_task].active = 1;

  return;
}


typedef struct accumulate_test_information
{
  pami_data_function data_fn[ACCUMULATE_TEST_COUNT];
  void *             data_cookie[ACCUMULATE_TEST_COUNT];
  struct iovec       data_buffer;
  double             scalar;
} accumulate_test_information_t;

void accumulate_test_done_cb (pami_context_t   context,
                              void           * cookie,
                              pami_result_t    result)
{
  accumulate_test_t test = (accumulate_test_t) cookie;
  fprintf (stdout, "(%03d) end accumulate test \"%s\"\n", __LINE__, accumulate_test_name[test]);
}

void accumulate_test_recv_cb (pami_context_t    context,
                              void            * cookie,
                              const void      * header_addr,
                              size_t            header_size,
                              const void      * pipe_addr,
                              size_t            data_size,
                              pami_endpoint_t   origin,
                              pami_recv_t     * recv)
{
  accumulate_test_t test = *((accumulate_test_t *) header_addr);
  fprintf (stdout, "(%03d) begin accumulate test \"%s\"\n", __LINE__, accumulate_test_name[test]);

  accumulate_test_information_t * info = (accumulate_test_information_t *) cookie;

  // ...

  recv->cookie      = (void *) test;
  recv->local_fn    = accumulate_test_done_cb;
  recv->type        = PAMI_TYPE_DOUBLE;
  recv->addr        = info->data_buffer.iov_base;
  recv->offset      = 0;
  recv->data_fn     = info->data_fn[test];
  recv->data_cookie = info->data_cookie[test];


  return;
}

void test_fn (int argc, char * argv[], pami_client_t client, pami_context_t context[])
{
  int num_doubles = 16;

  if (argc > 1) num_doubles = atoi (argv[1]);

  size_t num_tasks = size (client);
  pami_task_t my_task_id = task (client);
  pami_task_t target_task_id = num_tasks - 1;
  pami_task_t origin_task_id = 0;


  /*
   * Allocate a 'window' of memory region information, one for each task in the
   * client. Only the 'local' memory region information for this task will
   * contain a valid data buffer. The memory region information is marked
   * 'active' when the memory regions are received from each remote task.
   */
  memregion_information_t * mr_info =
    (memregion_information_t *) malloc (sizeof(memregion_information_t) * num_tasks);
  unsigned i;
  for (i = 0; i < num_tasks; i++)
    {
      mr_info[i].data.iov_len = 0;
      mr_info[i].data.iov_base = NULL;
      mr_info[i].active = 0;
    }


  /*
   * Create a local memregion for each context.
   *
   * Note that both memregions will describe the same memory location. This is
   * necessary when writing portable, platform independent code as the physical
   * hardware underlying the contexts may, or may not, require separate memory
   * pinning.
   */
  size_t actual_memregion_bytes = 0;
  mr_info[my_task_id].data.iov_base = malloc (sizeof(double) * num_doubles);
  mr_info[my_task_id].data.iov_len = sizeof(double) * num_doubles;
  PAMI_Memregion_create (context[0],
                         mr_info[my_task_id].data.iov_base,
                         mr_info[my_task_id].data.iov_len,
                         & actual_memregion_bytes,
                         & mr_info[my_task_id].memregion[0]);
  PAMI_Memregion_create (context[1],
                         mr_info[my_task_id].data.iov_base,
                         mr_info[my_task_id].data.iov_len,
                         & actual_memregion_bytes,
                         & mr_info[my_task_id].memregion[1]);
  mr_info[my_task_id].active = 1;


  /*
   * Register the memory region exchange dispatch; only needed on the
   * first context of each task.
   */
  pami_dispatch_hint_t mr_hint = {0};
  pami_dispatch_callback_function mr_dispatch;
  mr_dispatch.p2p = exchange_memregion_recv_cb;
  PAMI_Dispatch_set (context[0], MEMREGION_EXCHANGE_DISPATCH_ID, mr_dispatch, (void *) mr_info, mr_hint);



  accumulate_test_information_t test_info;

  test_info.data_buffer.iov_base = malloc (sizeof(double) * num_doubles);
  test_info.data_buffer.iov_len = sizeof(double) * num_doubles;
  test_info.scalar = 1.2;

  test_info.data_fn[ACCUMULATE_TEST_SCALAR_SUM] = accumulate_scalar_sum_data_function;
  test_info.data_cookie[ACCUMULATE_TEST_SCALAR_SUM] = (void *) & test_info.scalar;

  test_info.data_fn[ACCUMULATE_TEST_VECTOR_SUM] = accumulate_vector_sum_data_function;
  test_info.data_cookie[ACCUMULATE_TEST_VECTOR_SUM] = malloc (sizeof(double) * num_doubles);

  test_info.data_fn[ACCUMULATE_TEST_SCALAR_SUBTRACT] = accumulate_scalar_subtract_data_function;
  test_info.data_cookie[ACCUMULATE_TEST_SCALAR_SUBTRACT] = (void *) & test_info.scalar;

  test_info.data_fn[ACCUMULATE_TEST_VECTOR_SUBTRACT] = accumulate_vector_subtract_data_function;
  test_info.data_cookie[ACCUMULATE_TEST_VECTOR_SUBTRACT] = malloc (sizeof(double) * num_doubles);

  test_info.data_fn[ACCUMULATE_TEST_VECTOR_MAX_SUM] = accumulate_vector_max_sum_data_function;
  test_info.data_cookie[ACCUMULATE_TEST_VECTOR_MAX_SUM] = malloc (sizeof(double) * num_doubles);

  test_info.data_fn[ACCUMULATE_TEST_VECTOR_MIN_SUM] = accumulate_vector_min_sum_data_function;
  test_info.data_cookie[ACCUMULATE_TEST_VECTOR_MIN_SUM] = malloc (sizeof(double) * num_doubles);


  /*
   * Register the accumulate dispatch; needed on both
   * contexts to enable "crosstalk".
   */
  pami_dispatch_hint_t acc_hint = {0};
  acc_hint.recv_immediate = PAMI_HINT_DISABLE;
  pami_dispatch_callback_function acc_dispatch;
  acc_dispatch.p2p = accumulate_test_recv_cb;
  PAMI_Dispatch_set (context[0], ACCUMULATE_TEST_DISPATCH_ID, acc_dispatch, (void *) & test_info, acc_hint);
  PAMI_Dispatch_set (context[1], ACCUMULATE_TEST_DISPATCH_ID, acc_dispatch, (void *) & test_info, acc_hint);


  simple_barrier(client, context[0]);


  /*
   * Exchange the memory regions
   */
  volatile unsigned mr_exchange_active = 0;
  pami_send_t mr_exchange_parameters = {0};
  mr_exchange_parameters.send.dispatch = MEMREGION_EXCHANGE_DISPATCH_ID;
  mr_exchange_parameters.send.header.iov_base = NULL;
  mr_exchange_parameters.send.header.iov_len = 0;
  mr_exchange_parameters.send.data.iov_base = (void *) mr_info[my_task_id].memregion;
  mr_exchange_parameters.send.data.iov_len = sizeof(pami_memregion_t) * 2;
  mr_exchange_parameters.events.cookie = (void *) & mr_exchange_active;
  mr_exchange_parameters.events.local_fn = decrement;

  for (i = 0; i < num_tasks; i++)
    {
      if (i == my_task_id) continue;

      PAMI_Endpoint_create (client, i, 0, & mr_exchange_parameters.send.dest);
      mr_exchange_active++;
      PAMI_Send (context[0], & mr_exchange_parameters);
    }

  /*
   * Advance until local memory regions have been sent and
   * all memory regions have been received.
   */
  unsigned num_memregions_active;

  do
    {
      num_memregions_active = 0;

      for (i = 0; i < num_tasks; i++)
        num_memregions_active += mr_info[i].active;

      PAMI_Context_advance (context[0], 1);
    }
  while (num_memregions_active < num_tasks);

  while (mr_exchange_active > 0)
    PAMI_Context_advance (context[0], 1);

#ifdef ASYNC_PROGRESS
  async_progress_t async_progress;
  async_progress_open (client, &async_progress);
  async_progress_enable (&async_progress, context[1]);
#endif

  if (my_task_id == target_task_id)
    {
      /*
       * This is the "passive target" task.
       */
#ifdef ASYNC_PROGRESS
      /*
       * Do "something" besides communication for a little bit.
       */
      sleep(1);
#else
      /*
       * Advance the second context for a little bit.
       */
      fprintf (stdout, "(%03d) spoofing async progress\n", __LINE__);
      for (i=0; i<10; i++)
      {
        fprintf (stdout, "(%03d) 'async progress context' advancing\n", __LINE__);
        PAMI_Context_advance (context[1], 100000);
        fprintf (stdout, "(%03d) 'async progress context' sleeping\n", __LINE__);
        sleep(1);
      }
#endif
    }
  else if (my_task_id == origin_task_id)
    {
      /*
       * This is the "active origin" task.
       */

      {
        /*
         * Use rdma put to initialize the remote buffer with the local data.
         */
        volatile unsigned rput_active = 1;
        pami_rput_simple_t rput_parameters = {0};
        PAMI_Endpoint_create (client, target_task_id, 1, & rput_parameters.rma.dest);
        rput_parameters.rma.bytes = num_doubles * sizeof(double);
        rput_parameters.rdma.local.mr = mr_info[origin_task_id].memregion;
        rput_parameters.rdma.local.offset = 0;
        rput_parameters.rdma.remote.mr = mr_info[target_task_id].memregion;
        rput_parameters.rdma.remote.offset = 0;
        rput_parameters.put.rdone_fn = decrement;
        rput_parameters.rma.cookie = (void *) & rput_active;

        PAMI_Rput (context[0], & rput_parameters);

        while (rput_active > 0)
          PAMI_Context_advance (context[0], 1);
      }

      {
        volatile unsigned send_active = 0;
        accumulate_test_t test_id = ACCUMULATE_TEST_SCALAR_SUM;
        pami_send_t send_parameters = {0};
        PAMI_Endpoint_create (client, target_task_id, 1, & send_parameters.send.dest);
        send_parameters.send.dispatch = ACCUMULATE_TEST_DISPATCH_ID;
        send_parameters.send.header.iov_len = sizeof (accumulate_test_t);
        send_parameters.send.header.iov_base = (void *) & test_id;
        send_parameters.send.data.iov_base = test_info.data_buffer.iov_base;
        send_parameters.send.data.iov_len = test_info.data_buffer.iov_len;
        send_parameters.events.remote_fn = decrement;
        send_parameters.events.cookie = (void *) & send_active;

        for (test_id = ACCUMULATE_TEST_SCALAR_SUM; test_id < ACCUMULATE_TEST_COUNT; test_id++)
          {
            send_active = 1;

            fprintf (stdout, "(%03d) sending data buffer for accumulate test \"%s\"\n", __LINE__, accumulate_test_name[test_id]);
            PAMI_Send (context[0], & send_parameters);

            fprintf (stdout, "(%03d) waiting for remote completion of data buffer sent for accumulate test \"%s\"\n", __LINE__, accumulate_test_name[test_id]);
            while (send_active > 0)
              PAMI_Context_advance (context[0], 1);
            fprintf (stdout, "(%03d) data buffer received on remote for accumulate test \"%s\"\n", __LINE__, accumulate_test_name[test_id]);
          }
      }

      {
        /*
         * Use rdma get to retrieve the remote buffer and compare results.
         */
        volatile unsigned rget_active = 1;
        pami_rget_simple_t rget_parameters = {0};
        PAMI_Endpoint_create (client, target_task_id, 1, & rget_parameters.rma.dest);
        rget_parameters.rma.done_fn = decrement;
        rget_parameters.rma.cookie = (void *) & rget_active;
        rget_parameters.rma.bytes = sizeof(double) * num_doubles;
        rget_parameters.rdma.local.mr = mr_info[origin_task_id].memregion;
        rget_parameters.rdma.local.offset = 0;
        rget_parameters.rdma.remote.mr = mr_info[target_task_id].memregion;
        rget_parameters.rdma.remote.offset = 0;

        PAMI_Rget (context[0], & rget_parameters);

        while (rget_active > 0)
          PAMI_Context_advance (context[0], 1);
      }

    }
  else
    {
      /*
       * All other tasks, if any, do nothing and simply enter the barrier.
       */
    }

  simple_barrier (client, context[0]);

#ifdef ASYNC_PROGRESS
  async_progress_disable (&async_progress, context[1]);
  async_progress_close (&async_progress);
#endif


  /*
   * Do cleanup ?
   */


  return;
}
