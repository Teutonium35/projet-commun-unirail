#ifndef _UNIRAIL_POSITION_H
	#define _UNIRAIL_POSITION_H

	#include "../../utility/include/comm.h"
	#include "../../utility/include/map.h"


	/**
	 * @struct report_position_args_t
	 * @brief Arguments to pass to the report_position thread function
	 * @param client The client struct to use for communication
	 * @param train_id The ID of the train to report the position of
	 * @param pos The position to report
	 * @param pos_mutex The mutex to lock when accessing the position
	 * @param pos_initialized Flag to indicate if the position has been initialized
	 * @param init_pos_cond The condition to signal when the position is initialized
	 * @param init_pos_mutex The mutex to lock when accessing the initialization condition
	 */
	typedef struct {
		client_udp_init_t client;
		int train_id;
		position_t * pos;
		pthread_mutex_t * pos_mutex;
		int * pos_initialized;
		pthread_cond_t * init_pos_cond;
		pthread_mutex_t * init_pos_mutex;
	} report_position_args_t;

	void * report_position(void * args);
#endif