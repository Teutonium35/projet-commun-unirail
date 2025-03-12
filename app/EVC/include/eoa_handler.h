#ifndef _UNIRAIL_EOA_H
	#define _UNIRAIL_EOA_H

	#include "../../utility/include/comm.h"
	#include "../../utility/include/map.h"

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
	 * @var pos_inittialized Flag to indicate if the position has been initialized
	 * @var init_pos_cond The condition that signals when the position is initialized
	 * @var init_pos_mutex The mutex to lock when accessing the initialization condition
	 * @var eoa_inittialized Flag to indicate if the eoa has been initialized
	 * @var init_eoa_cond The condition to signal when the eoa is initialized
	 * @var init_eoa_mutex The mutex to lock when accessing the eoa initialization condition
	 */
	typedef struct {
		client_udp_init_t client;
		int train_id;
		int chemin_id;
		position_t * eoa;
		pthread_mutex_t * eoa_mutex;
		position_t * pos;
		pthread_mutex_t * pos_mutex;
		int * pos_initialized;
		pthread_cond_t * init_pos_cond;
		pthread_mutex_t * init_pos_mutex;
		int * eoa_initialized;
		pthread_cond_t * init_eoa_cond;
		pthread_mutex_t * init_eoa_mutex;
	} eoa_handler_args_t;

	void * eoa_handler(void * args);
#endif