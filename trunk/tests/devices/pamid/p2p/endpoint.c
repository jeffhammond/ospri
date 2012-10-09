#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <pami.h>

//#define SLEEP sleep
#define SLEEP usleep

int main(int argc, char* argv[])
{
  pami_result_t result = PAMI_ERROR;

  char * client1name = "";
  pami_client_t client1;
  result = PAMI_Client_create(client1name, &client1, NULL, 0);
  assert(result == PAMI_SUCCESS);

  char * client2name = "";
  pami_client_t client2;
  result = PAMI_Client_create(client2name, &client2, NULL, 0);
  assert(result == PAMI_SUCCESS);

  pami_endpoint_t ep1;
  PAMI_Endpoint_create(client1, (pami_task_t)0, 0, &ep1);

  pami_endpoint_t ep2;
  PAMI_Endpoint_create(client2, (pami_task_t)0, 0, &ep2);

  printf("ep1 = %d, ep2 = %d \n", ep1, ep2);
  fflush(stdout);
  SLEEP(1);

  result = PAMI_Client_destroy(&client2);
  assert(result == PAMI_SUCCESS);

  result = PAMI_Client_destroy(&client1);
  assert(result == PAMI_SUCCESS);

  printf("end of test \n");
  fflush(stdout);
  SLEEP(1);

  return 0;
}

