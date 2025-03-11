#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/eoa_handler.h"
#include "../include/automatique.h"
#include "../include/map.h"

void * eoa_handler(void * args) {
	eoa_handler_args_t * eha = (eoa_handler_args_t *) args;
	position_t local_pos;
	message_t send_message, recv_message;
	char * data[MAXDATA];
	char pos_str[50];
	int nbcar;
	int request_new_eoa = 0;

	const int distance_triggers[] = { 5000, 2500, 100 };
	const int distance_triggers_size = sizeof(distance_triggers) / sizeof(int);

	double distance;
	double last_distance_triggered = distance_triggers[0];

	while (1) {
		pthread_mutex_lock(eha->pos_mutex);
		local_pos.bal = eha->pos->bal;
		local_pos.pos_r = eha->pos->pos_r;
		pthread_mutex_unlock(eha->pos_mutex);

		if (local_pos.bal  != 0) {
			pthread_mutex_lock(eha->eoa_mutex);
			distance = get_distance(local_pos, *(eha->eoa), eha->chemin_id);
			pthread_mutex_unlock(eha->eoa_mutex);

			printf("EVC [%d] - Distance jusqu'à l'EOA : %.2f\n", eha->train_id, distance);
			fflush(stdout);

			// Sends a request once when the train reaches each threshold, or continuously if it is past the last threshold
			for (int i = 0; i < distance_triggers_size - 1; i++) {
				if (distance < distance_triggers[i] && last_distance_triggered >= distance_triggers[i]) {
					request_new_eoa = 1;
				}
			}
			if (distance < distance_triggers[distance_triggers_size - 1]) {
				request_new_eoa = 1;
			}

			if (request_new_eoa) {
				printf("EVC [%d] - Envoi d'une nouvelle demande d'autorisation de mouvement\n", eha->train_id);
				send_message.id = eha->train_id;
				send_message.code = 102;
				for (int i = 0; i < MAXDATA; i++) {
					send_message.data[i] = NULL;
				}

				send_data(eha->client.sd, eha->client.adr_serv, send_message);

				request_new_eoa = 0;
				last_distance_triggered = distance;

				//receive_data(eha->client.sd, NULL, &recv_message);
			}
		}

		usleep(1000000);
	}

	pthread_exit(NULL);

	return NULL;
}