#include <stdio.h>
#include <stdlib.h>
#include "../include/requests_handler.h"
#include "../include/trains.h"
#include "../include/ressources.h"
#include "../../utility/include/const_chemins.h"

void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains);
void free_resources(int * next_bal_index, int no_train, position_t * pos_trains);

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
			free_resources(&next_bal_index_lib[recv_message.id],recv_message.id, pos_trains);
			pthread_mutex_unlock(&pos_trains_locks[recv_message.id]);


			send_message->code = 200;
			send_message->data[0] = NULL;
			break;

		case 102:
			printf("RBC [%d] - Demande d'autorisation de mouvement reçu\n", recv_message.id);
			ask_resources(&next_bal_index_req[recv_message.id],recv_message.id, pos_trains);

			printf("Ressource accordée : %d", next_bal_index_req[recv_message.id]);

			send_message->code = 200;
			send_message->data[0] = NULL;

			break;
			
	   	default:
			printf("RBC - Code de requête inconnu\n");
			send_message->code = 400;
			send_message->data[0] = NULL;
			break;
   }
}

void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains){
	pthread_mutex_lock(&pos_trains_locks[no_train]);
	if ((pos_trains[no_train].bal == L_res_req[no_train][*next_bal_index]) && (pos_trains[no_train].pos_r >= 0)){
		lock_ressources(L_mask_req[no_train][*next_bal_index]);

		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		if (*next_bal_index == (sizeof(L_res_req[no_train])/sizeof(int))){
			*next_bal_index = 0;
		}
		// Sinon, c'est la prochaine dans le chemin
		else {
			*next_bal_index += 1;
		}
	}
	pthread_mutex_unlock(&pos_trains_locks[no_train]);
}

void free_resources(int * next_bal_index, int no_train, position_t * pos_trains){
	if ((pos_trains[no_train].bal == L_res_lib[no_train][*next_bal_index]) && (pos_trains[no_train].pos_r >= 0)){
		unlock_ressources(L_mask_lib[no_train][*next_bal_index]);


		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		if (*next_bal_index == (sizeof(L_res_req[no_train])/sizeof(int))){
			*next_bal_index = 0;
		}
		// Sinon, c'est la prochaine dans le chemin
		else {
			*next_bal_index += 1;
		}
	}
}