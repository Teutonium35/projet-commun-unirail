#ifndef _UNIRAIL_RESPONSE_LISTENER_H
	#define _UNIRAIL_RESPONSE_LISTENER_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <time.h>
	#include <sys/time.h>
	#include <errno.h>

	#include "../../utility/include/comm.h"
	#include "../../utility/include/comm_message_listener.h"


	typedef struct {
		int sd;
		pthread_mutex_t * mission_mutex;
		int * mission;
		pending_message_t pending_request;
	} requests_handler_args_t;

	void * requests_handler(void * args);

	void handle_request(message_t recv_message, message_t * send_message, pthread_mutex_t * mission_mutex, int * mission);

#endif