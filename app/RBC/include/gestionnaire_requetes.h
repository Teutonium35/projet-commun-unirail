#ifndef _UNIRAIL_GESTION_REQ_H_
	#define _UNIRAIL_GESTION_REQ_H_
	#include "comm.h"

	/**
	 * @brief Handles a request and prepares a response
	 * @param recv_message The message that was received to handle
	 * @param send_message The message in which to store the response
	 */
	void handle_request(message_t recv_message, message_t * send_message);
#endif