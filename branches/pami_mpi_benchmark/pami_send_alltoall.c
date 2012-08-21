#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

#include <pami.h>

#define ITERATIONS 10

#define WARMUP

#define BUFSIZE 8*1024*1024

#define MIN_DATA_LEN 1

#define THRESHOLD 128

#define MSGCOUNT 1

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) /* fprintf x */
#endif

volatile unsigned _recv_active;
uint8_t _tmpbuffer[BUFSIZE];
size_t _my_rank;

/* --------------------------------------------------------------- */

static void decrement (pami_context_t   context,
		void          * cookie,
		pami_result_t    result)
{
	unsigned * value = (unsigned *) cookie;
	TRACE_ERR((stderr, "(%zu) decrement() cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
	--*value;
}

/* --------------------------------------------------------------- */
static void test_dispatch (
		pami_context_t        context,      /**< IN: PAMI context */
		void               * cookie,       /**< IN: dispatch cookie */
		const void         * header_addr,  /**< IN: header address */
		size_t               header_size,  /**< IN: header size */
		const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
		size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
		pami_endpoint_t origin,
		pami_recv_t         * recv)        /**< OUT: receive message structure */
{
	//if(_my_rank != 0)
	//	usleep(1);
	unsigned * value = (unsigned *) cookie;
	//printf("%d received with cookie = %d\n", _my_rank, *value);
	if (pipe_addr != NULL)
	{
		TRACE_ERR((stderr, "(%zu) short recv:  decrement cookie = %p, %d => %d\n", _my_rank, cookie, *value, *value-1));
		memcpy((void *)_tmpbuffer, pipe_addr, pipe_size);
		--*value;
		//printf("%d received with updated cookie = %d, _recv_active = %d\n", _my_rank, *value, _recv_active);
		return;
	}
	
	TRACE_ERR((stderr, "(%zu) long recvn", _my_rank));
	recv->local_fn = decrement;
	recv->cookie   = cookie;
	recv->type     = PAMI_TYPE_BYTE;
	recv->addr     = (void *)_tmpbuffer;
	recv->offset   = 0;;
	
}

unsigned long long test (pami_client_t client, pami_context_t context, size_t dispatch, size_t hdrlen, size_t sndlen, size_t myrank, size_t ntasks)
{
	TRACE_ERR((stderr, "(%zu) Do test ... sndlen = %zu\n", myrank, sndlen));

	char metadata[BUFSIZE];
	char buffer[BUFSIZE];
	unsigned i, j;

	_recv_active = (ntasks - 1)* ITERATIONS;
	
	volatile unsigned send_active = MSGCOUNT * ITERATIONS;

	pami_endpoint_t endpoint[ntasks];
	for (i=0; i<ntasks; i++)
		PAMI_Endpoint_create (client, i, 0, &endpoint[i]);

	pami_send_t parameters;
	parameters.send.dest = endpoint[0];
	parameters.send.dispatch = dispatch;
	parameters.send.header.iov_base = metadata;
	parameters.send.header.iov_len = hdrlen;
	parameters.send.data.iov_base  = buffer;
	parameters.send.data.iov_len = sndlen;
	parameters.events.cookie        = (void *) &send_active;
	parameters.events.local_fn      = NULL;//decrement;
	parameters.events.remote_fn     = NULL;
	
	/*barrier (); */
	usleep(1000);

	unsigned long long t1 = PAMI_Wtimebase(client);
	for(int iter = 0; iter < ITERATIONS; iter++) {
		for(int i = 0; i < ntasks; i++) {
			if(myrank != i) {
				parameters.send.dest = endpoint[i];
				if(sndlen+hdrlen > THRESHOLD) {
					PAMI_Send (context, &parameters);
				} else {
					PAMI_Send_immediate (context, &parameters.send);
				}
			}
		}
	}
	
	while(_recv_active) {
		PAMI_Context_advance(context, 100);
	}
	
	unsigned long long t2 = PAMI_Wtimebase(client);

	return (t2-t1)/ITERATIONS;
}

int main (int argc, char ** argv)
{
	TRACE_ERR((stderr, "Start test ...\n"));

	size_t hdrcnt = argc;
	size_t hdrsize[1024];
	hdrsize[0] = 0;

	int arg;
	for (arg=1; arg<argc; arg++)
	{
		hdrsize[arg] = (size_t) strtol (argv[arg], NULL, 10);
	}

	pami_client_t client;
	char clientname[]="TEST";
	TRACE_ERR((stderr, "... before PAMI_Client_create()\n"));
	PAMI_Client_create (clientname, &client, NULL, 0);
	TRACE_ERR((stderr, "...  after PAMI_Client_create()\n"));
	pami_context_t context;
	TRACE_ERR((stderr, "... before PAMI_Context_createv()\n"));
	PAMI_Context_createv (client, NULL, 0, &context, 1);
	TRACE_ERR((stderr, "...  after PAMI_Context_createv()\n"));

	/*TRACE_ERR((stderr, "... before barrier_init()\n")); */
	/*barrier_init (client, context, 0); */
	/*TRACE_ERR((stderr, "...  after barrier_init()\n")); */


	/* Register the protocols to test */
	size_t dispatch = 1;
	pami_dispatch_callback_function fn;
	fn.p2p = test_dispatch;
	pami_dispatch_hint_t options={};
	TRACE_ERR((stderr, "Before PAMI_Dispatch_set() .. &_recv_active = %p, recv_active = %zu\n", &_recv_active, _recv_active));
	pami_result_t result = PAMI_Dispatch_set (context,
			dispatch,
			fn,
			(void *)&_recv_active,
			options);
	if (result != PAMI_SUCCESS)
	{
		fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
		return 1;
	}

	pami_configuration_t configuration;

	configuration.name = PAMI_CLIENT_TASK_ID;
	result = PAMI_Client_query(client, &configuration,1);
	_my_rank = configuration.value.intval;

	configuration.name = PAMI_CLIENT_NUM_TASKS;
	result = PAMI_Client_query(client, &configuration,1);
	size_t num_tasks = configuration.value.intval;

	configuration.name = PAMI_CLIENT_WTICK;
	result = PAMI_Client_query(client, &configuration,1);
	double tick = configuration.value.doubleval;

	/* Display some test header information */
	if (_my_rank == 0)
	{
		char str[2][1024];
		int index[2];
		index[0] = 0;
		index[1] = 0;

		index[0] += sprintf (&str[0][index[0]], "#          ");
		index[1] += sprintf (&str[1][index[1]], "#    bytes ");

		fprintf (stdout, "# send flood performance test\n");
		fprintf (stdout, "#   Number of tasks 'flooding' task 0: %zu\n", num_tasks-1);
		fprintf (stdout, "#\n");

		unsigned i;
		for (i=0; i<hdrcnt; i++)
		{
			if (i==0)
				fprintf (stdout, "# testcase %d : header bytes = %3zd\n", i, hdrsize[i]);
			else
				fprintf (stdout, "# testcase %d : header bytes = %3zd (argv[%d])\n", i, hdrsize[i], i);
			index[0] += sprintf (&str[0][index[0]], "[- testcase %d -] ", i);
			index[1] += sprintf (&str[1][index[1]], " cycles    usec    bandwidth");
		}

		fprintf (stdout, "#\n");
		fprintf (stdout, "%s\n", str[0]);
		fprintf (stdout, "%s\n", str[1]);
		fflush (stdout);
	}

	/*barrier (); */

	unsigned long long cycles;
	double usec, bandwidth;

	char str[10240];
	size_t sndlen = MIN_DATA_LEN;

	for (; sndlen <= BUFSIZE; sndlen = sndlen*2)
	{
		int index = 0;
		index += sprintf (&str[index], "%10zd ", sndlen);

		unsigned i;
		for (i=0; i<hdrcnt; i++)
		{

#ifdef WARMUP
			test (client, context, dispatch, hdrsize[i], sndlen, _my_rank, num_tasks);
#endif
			cycles = test (client, context, dispatch, hdrsize[i], sndlen, _my_rank, num_tasks);
			usec   = cycles * tick * 1000000.0;
			bandwidth = sndlen*1000000.0/(usec*1024*1024);
			index += sprintf (&str[index], "%7lld %7.4f %8.6f ", cycles, usec, bandwidth);
		}

		if (_my_rank == 0) {
			fprintf (stdout, "%s\n", str);
			fflush(stdout);
		}
	}

	PAMI_Client_destroy(&client);

	return 0;
}
#undef TRACE_ERR
