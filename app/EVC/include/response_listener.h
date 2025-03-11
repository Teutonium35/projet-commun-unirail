#ifndef _UNIRAIL_RESPONSE_LISTENER_H
	#define _UNIRAIL_RESPONSE_LISTENER_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>

	#include "../../utility/include/comm.h"

	/**
	 * @brief Awaits a response to a given request
	 * @param req_id The ID of the sent request to await the response for
	 * @param recv_message The message struct in which the received message will be stored
	 */
	void wait_for_response(int req_id, message_t *recv_message);

	/**
	 * @brief Dispatches a received message to the appropriate thread
	 * @param recv_message The received message to dispatch
	 */
	void dispatch_message(message_t *recv_message);

	/**
	 * @struct response_t
	 * @brief Linked list structure to store waiting responses
	 * @var response_t::req_id The ID of the request to await the response for
	 * @var response_t::message The message struct in which the received message will be stored
	 * @var response_t::ready Flag to indicate if the response is ready to be read
	 * @var response_t::cond Thread condition to signal when the response is ready
	 * @var response_t::mutex Thread mutex to lock the response struct
	 * @var response_t::next Pointer to the next response in the linked list
	 */
	typedef struct response_t {
		int req_id;
		message_t message;
		int ready;
		pthread_cond_t cond;
		pthread_mutex_t mutex;
		struct response_t *next;
	} response_t;

	/**
	 * @struct response_listener_args_t
	 * @brief Structure to pass arguments to the response listener thread
	 * @var response_listener_args_t::client The client struct to use for communication
	 */
	typedef struct {
		client_udp_init_t client;
	} response_listener_args_t;

	/**
	 * @brief Entry point for the response listener thread
	 * @param args The response_listener_args_t struct containing the client struct
	 */
	void *response_listener(void * args);

#endif