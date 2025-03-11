#ifndef _UNIRAIL_POSITION_H
	#define _UNIRAIL_POSITION_H

	#include "../../utility/include/comm.h"
	#include "../../utility/include/map.h"


	/**
	 * @struct report_position_args_t
	 * @brief Arguments to pass to the report_position thread function
	 * @var client The client struct to use for communication
	 * @var train_id The ID of the train to report the position of
	 * @var pos The position to report
	 * @var pos_mutex The mutex to lock when accessing the position
	 */
	typedef struct {
		client_udp_init_t client;
		int train_id;
		position_t * pos;
		pthread_mutex_t * pos_mutex;
	} report_position_args_t;

	void * report_position(void * args);
#endif