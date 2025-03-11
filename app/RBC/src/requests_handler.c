#include <stdio.h>
#include <stdlib.h>
#include "../include/requests_handler.h"
#include "../include/trains.h"

void handle_request(message_t recv_message, message_t * send_message) {
	send_message->train_id = recv_message.train_id;
	send_message->req_id = recv_message.req_id;

	if (recv_message.train_id >= NB_TRAINS) {
		printf("RBC - ID de train inconnu\n");
		send_message->code = 404;
		send_message->data[0] = NULL;
		return;
	}

   	switch (recv_message.code) {

		// Rapport de position
	   	case 101:
			printf("RBC [%d] - Rapport de position reçu\n", recv_message.train_id);

			char * endptr; // Pointeur pour vérifier que la conversion s'est bien passée

			long bal = strtol(recv_message.data[0], &endptr, 10);
			if (endptr == recv_message.data[0]) { // Le pointeur n'a pas bougé, la conversion a échoué
				send_message->code = 401;
				send_message->data[0] = NULL;
				break;
			}

			double pos_r = strtod(recv_message.data[1], &endptr);
			if (endptr == recv_message.data[1]) {
				send_message->code = 401;
				send_message->data[0] = NULL;
				break;
			}

			pthread_mutex_lock(&pos_trains_locks[recv_message.train_id - 1]);
			pos_trains[recv_message.train_id - 1].bal = bal;
			pos_trains[recv_message.train_id - 1].pos_r = atof(recv_message.data[1]);
			pthread_mutex_unlock(&pos_trains_locks[recv_message.train_id - 1]);

			send_message->code = 201;
			send_message->data[0] = NULL;
			break;

		case 102:
			printf("RBC [%d] - Demande d'autorisation de mouvement reçue\n", recv_message.train_id);
			// TODO
			send_message->code = 202;
			send_message->data[0] = NULL;
			break;
			
	   	default:
			printf("RBC - Code de requête inconnu\n");
			send_message->code = 400;
			send_message->data[0] = NULL;
			break;
   }
}