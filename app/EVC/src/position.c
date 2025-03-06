#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/position.h"

void * report_position(void * args) {
	report_position_args_t * rpa = (report_position_args_t *) args;
	position_t local_pos;
	message_t send_message;
	char * data[MAXDATA];
	char pos_str[50];
	int nbcar;

	while (1) {
		pthread_mutex_lock(rpa->pos_mutex);
		local_pos.bal = rpa->pos->bal;
		local_pos.pos_r = rpa->pos->pos_r;
		pthread_mutex_unlock(rpa->pos_mutex);

		data[0] = malloc(10);
		data[1] = malloc(10);

		sprintf(data[0], "%d", local_pos.bal);
		sprintf(data[1], "%f", local_pos.pos_r);

		send_message.id = rpa->train_id;
		send_message.code = 101;
		for (int i = 0; i < MAXDATA; i++) {
			send_message.data[i] = data[i];
		}

		send_data(rpa->client.sd, rpa->client.adr_serv, send_message);

		usleep(1000000);
	}

	pthread_exit(NULL);

	return NULL;
}