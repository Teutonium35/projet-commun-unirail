#include <stdio.h>
#include <stdlib.h>
#include "../include/requests_handler.h"
#include "../include/trains.h"

void handle_request(message_t recv_message, message_t * send_message) {
	send_message->id = recv_message.id;

	if (recv_message.id >= NB_TRAINS) {
		printf("RBC - ID de train inconnu\n");
		send_message->code = 404;
		send_message->data[0] = NULL;
		return;
	}

   	switch (recv_message.code) {

		// Rapport de position
	   	case 101:
			printf("RBC [%d] - Rapport de position reçu\n", recv_message.id);

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

			pthread_mutex_lock(&pos_trains_locks[recv_message.id]);
			pos_trains[recv_message.id].bal = bal;
			pos_trains[recv_message.id].pos_r = atof(recv_message.data[1]);
			pthread_mutex_unlock(&pos_trains_locks[recv_message.id]);

			send_message->code = 200;
			send_message->data[0] = NULL;
			break;

		case 102:
			printf("RBC [%d] - Demande d'autorisation de mouvement reçu\n", recv_message.id);
			// TODO
			break;
			
	   	default:
			printf("RBC - Code de requête inconnu\n");
			send_message->code = 400;
			send_message->data[0] = NULL;
			break;
   }
}