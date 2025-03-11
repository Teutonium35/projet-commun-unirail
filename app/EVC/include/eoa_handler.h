#ifndef _UNIRAIL_EOA_H
	#define _UNIRAIL_EOA_H

	#include "../../utility/include/comm.h"
	#include "../include/position.h"

	/**
	 * @struct eoa_handler_args_t
	 * @brief Arguments to pass to the eoa handler thread function
	 * @var client The client struct to use for communication
	 * @var train_id The ID of the train to track the eoa of
	 * @var chemin_id The ID of the track to use
	 * @var eoa The current end of authorization
	 * @var eoa_mutex The mutex to lock when accessing the eoa
	 * @var pos The current position of the train
	 * @var pos_mutex The mutex to lock when accessing the position
	 */
	typedef struct {
		client_udp_init_t client;
		int train_id;
		int chemin_id;
		position_t * eoa;
		pthread_mutex_t * eoa_mutex;
		position_t * pos;
		pthread_mutex_t * pos_mutex;
	} eoa_handler_args_t;

	void * eoa_handler(void * args);
#endif