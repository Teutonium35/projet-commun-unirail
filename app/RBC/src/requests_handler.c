#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "../../utility/include/map.h"
#include "../include/requests_handler.h"
#include "../include/ressources.h"
#include "../include/trains.h"
#include "../include/eoa_calculator.h"
#include "../../utility/include/const_chemins.h"
#include "../../utility/include/map.h"
#include "../../utility/include/can.h"
#include "../../utility/include/can_infra.h"

void * handle_and_respond(void * args) {
	handle_and_respond_args_t * hra = (handle_and_respond_args_t *) args;
	message_t send_message;

	handle_request(hra->recv_message, &send_message, hra->can_socket);

	send_data(hra->sd, *hra->client_adr, send_message);

	free(hra->client_adr);
	free(hra);

	pthread_exit(NULL);

	return NULL;
}

void handle_request(message_t recv_message, message_t * send_message, int can_socket) {

	send_message->train_id = recv_message.train_id;
	send_message->req_id = recv_message.req_id;

	if (recv_message.train_id - 1 >= NB_TRAINS) {
		printf("RBC - ID de train inconnu\n");
		send_message->code = 404;
		send_message->data[0] = NULL;
		return;
	}

   	switch (recv_message.code) {

		// Rapport de position
	   	case 101:
			printf("RBC [%d] - Rapport de position reçu\n", recv_message.train_id-1);

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
			free_resources(&next_bal_index_lib[recv_message.train_id - 1],recv_message.train_id - 1, pos_trains);
			if (DEBUG_RES_FREE){
				printf("RBC [%d] - Prochaine ressource libérée : %d\n", recv_message.train_id - 1, next_bal_index_lib[recv_message.train_id - 1]);
				printf("RBC [%d] - Ressources libres : %d\n", recv_message.train_id - 1, resources);
			}
			pthread_mutex_unlock(&pos_trains_locks[recv_message.train_id - 1]);


			send_message->code = 201;
			send_message->data[0] = NULL;
			break;

		case 102:
			printf("RBC [%d] - Demande d'autorisation de mouvement reçue\n", recv_message.train_id - 1);
			ask_resources(&next_bal_index_req[recv_message.train_id - 1],recv_message.train_id - 1, pos_trains, can_socket);


			if (DEBUG_RES){
				printf("RBC [%d] - Ressource accordée : %d\n", recv_message.train_id - 1, next_bal_index_req[recv_message.train_id - 1]);
				printf("RBC [%d] - Ressources libres : %d\n", recv_message.train_id - 1, resources);
			}

			position_t EOA = next_eoa(recv_message.train_id-1,pos_trains,L_res_req[recv_message.train_id - 1][next_bal_index_req[recv_message.train_id - 1]], chemins, tailles_chemins);
			if (DEBUG_EOA){
				printf("RBC [%d] - Prochaine EOA pour train %d: balise %d, position %f\n", recv_message.train_id - 1, recv_message.train_id - 1, EOA.bal, EOA.pos_r);
			}

			send_message->code = 202;
			
			char * data[2];

			data[0] = malloc(10);
			data[1] = malloc(10);

			snprintf(data[0], 10, "%d", EOA.bal);
			snprintf(data[1], 10, "%2.f", EOA.pos_r);


			for (int i = 0; i < 2; i++) {
				send_message->data[i] = data[i];
			}
			break;

		case 100:
			send_message->code = 200;
			send_message->data[0] = NULL;
			send_message->data[1] = NULL;

			break;
			
	   	default:
			printf("RBC - Code de requête inconnu\n");
			send_message->code = 400;
			send_message->data[0] = NULL;
			break;
   }
}