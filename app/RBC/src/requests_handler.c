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
#include "../../utility/include/const_chemins.h"
#include "../../utility/include/map.h"
#include "../../utility/include/can.h"
#include "../../utility/include/can_infra.h"

void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains, int can_socket);
void free_resources(int * next_bal_index, int no_train, position_t * pos_trains);
position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins);

#define DEBUG_EOA 1
#define DEBUG_RES 1
#define DEBUG_POS 1
#define DEBUG_RES_FREE 1
#define DEBUG_AIG 1


void handle_request(message_t recv_message, message_t * send_message, int can_socket) {
	send_message->train_id = recv_message.train_id;
	send_message->req_id = recv_message.req_id;

	if (DEBUG_POS){
		printf("Id du train: %d\n", recv_message.train_id);
	}

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

			if (DEBUG_POS){
				printf("RBC [%d] - Position : bal %ld, pos %lf\n", recv_message.train_id - 1, bal, pos_r);
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

void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains, int can_socket){
	pthread_mutex_lock(&pos_trains_locks[no_train]);
	if (DEBUG_RES){
		printf("RBC [%d] - \n\nDans demande ressource\nPosition bal: %d, pos: %f\n",no_train, pos_trains[no_train].bal, pos_trains[no_train].pos_r);
		printf("RBC [%d] - Indice de la prochaine ressource : %d \nRessource à verrouiller : %d\n",no_train, *next_bal_index, L_mask_req[no_train][*next_bal_index]);
		printf("RBC [%d] - Balise actuelle: %d, balise à attendre pour demande: %d | %d | %d\n",no_train,pos_trains[no_train].bal, L_res_req[no_train][(*next_bal_index - 1 + tailles_chemins[no_train])%tailles_chemins[no_train]], no_train, (*next_bal_index - 1 + tailles_chemins[no_train])%tailles_chemins[no_train]);
	}

	// Si la position du train correspond à la position de demande de la prochaine ressource
	// On veut vérifier : bal actuelle = bal de ressource - 1
	if (DEBUG_RES){
		printf("RBC [%d] - Taille chemin %d: %d\n",no_train, no_train, tailles_chemins[no_train]);
	}
	int resource_to_lock = 0;
	for (int i=0;i<tailles_chemins[no_train];i++){
		if (DEBUG_RES){
			printf("RBC [%d] - Dans loop\n",no_train);
		}
		// donc bal actuelle = bal d'indice i - i
		int condition_1 = (pos_trains[no_train].bal == chemins[no_train][(i-1 + tailles_chemins[no_train])%tailles_chemins[no_train]]);
		// ou bal actuelle = bal d'indice i
		int condition_1bis = (pos_trains[no_train].bal == chemins[no_train][i]);
		// ET bal de ressource = bal d'indice i
		int condition_2 = (L_res_req[no_train][*next_bal_index] == chemins[no_train][i]);
		if (DEBUG_RES){
			printf("RBC [%d] - Condition 1: %d 1bis: %d 2: %d\n",no_train, condition_1, condition_1bis, condition_2);
			printf("RBC [%d] - Balise actuelle %d\nBalise d'indice i - 1 %d\nBalise de ressource %d\nBalise d'indice i %d\n",no_train, pos_trains[no_train].bal, chemins[no_train][(i-1 + tailles_chemins[no_train])%tailles_chemins[no_train]], L_res_req[no_train][*next_bal_index], chemins[no_train][i]);
		}

		if ((condition_1 || condition_1bis) && condition_2){
			resource_to_lock = 1;
			if (DEBUG_RES){
				printf("RBC [%d] - La ressource doit être verrouillée\n",no_train);
			}
			break;
		}
	}
	if (resource_to_lock && (pos_trains[no_train].pos_r >= 0)){
		if (DEBUG_RES){
			printf("RBC [%d] - Ressources avant lock: %d\n",no_train, resources);
			printf("RBC [%d] - Ressource à lock: %d\n",no_train, L_mask_req[no_train][*next_bal_index]);
		}
		lock_ressources(L_mask_req[no_train][*next_bal_index]);
		if (DEBUG_RES){
			printf("RBC [%d] - Ressources après lock: %d\n",no_train, resources);
		}

		if (DEBUG_RES){
			printf("RBC [%d] - Test condition :\n Indice actuel %d\n Limite %ld\n",no_train, *next_bal_index, sizeof(L_res_req[no_train])/sizeof(int));
		}

		if (DEBUG_AIG){
			printf("RBC [%d] - Setting switches\n",no_train);
		}
		set_all_switch(no_train, *next_bal_index, can_socket);
		if (DEBUG_AIG){
			printf("RBC [%d] - Switches set\n",no_train);
		}

		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		// Sinon, c'est la prochaine dans le chemin
		*next_bal_index = (*next_bal_index + 1)%L_res_size[no_train];
	}
	
	if (DEBUG_RES){
		printf("RBC [%d] - Indice de la prochaine ressource : %d \nProchaine ressource à verrouiller : %d\n",no_train, *next_bal_index, L_mask_req[no_train][*next_bal_index]);

		printf("RBC [%d] - Sortie demande ressource\n\n",no_train);
	}
	pthread_mutex_unlock(&pos_trains_locks[no_train]);
}

void free_resources(int * next_bal_index, int no_train, position_t * pos_trains){
	if ((pos_trains[no_train].bal == L_res_lib[no_train][*next_bal_index]) && (pos_trains[no_train].pos_r >= 0)){
		unlock_ressources(L_mask_lib[no_train][*next_bal_index]);
		if (DEBUG_RES_FREE){
			printf("Balise %d, libère ressource %d\n", pos_trains[no_train].bal, L_mask_lib[no_train][*next_bal_index]);
		}
		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		// Sinon, c'est la prochaine dans le chemin
		*next_bal_index = (*next_bal_index + 1)%L_res_size[no_train];
	}
}

/** 
 *	@brief Renvoie l'EOA du train num_train, en prenant en compte les autres trains ainsi que la prochaine balise avant une ressource.
 *	@param num_train numero du train entre 0 et 2
 *	@param pos_trains ensemble des trois positions des trains
 *	@param next_balise_avant_ressource numero de la prochaine balise sur le chemin du train avant que celui-ci ne doive demander une ressource
 *	@param chemins ensembles des 3 chemins des 3 trois (composée d'une liste de balise signalant leur ordre)
 *	@param len_chemins longueur des 3 chemins de "chemins"
 *	@return Renvoie l'EOA
**/
position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins){
	pthread_mutex_lock(&pos_trains_locks[1]);
	pthread_mutex_lock(&pos_trains_locks[2]);
	pthread_mutex_lock(&pos_trains_locks[3]);
	if (DEBUG_EOA){
	printf("Entrée dans next EOA\nNo train: %d\n Pos train actuel: %d, %f\nProchaine balise avant ressource: %d\n", num_train, pos_trains[num_train].bal, pos_trains[num_train].pos_r, next_balise_avant_ressource);
	}
	float distance_secu = 200.0;
    int i = 0; 
	float d;
    position_t current_position = pos_trains[num_train];
	position_t pos_balise = (position_t) {next_balise_avant_ressource,0.0};
	position_t pos_t2 = (position_t) {pos_trains[(num_train+1)%3].bal,pos_trains[(num_train+2)%3].pos_r - distance_secu};
	position_t pos_t3 = (position_t) {pos_trains[(num_train+2)%3].bal,pos_trains[(num_train+3)%3].pos_r - distance_secu};
    const int * chemin = chemins[num_train];
    int len_chemin = len_chemins[num_train];
	int train2_on_tracks = 0;
	int train3_on_tracks = 0;
    // On se place sur la balise de la position actuelle
	while(chemin[i] != len_chemin){ 
		if(pos_t2.bal == chemin[i]) train2_on_tracks = 1;
		if(pos_t3.bal == chemin[i]) train3_on_tracks = 1;
		i++;
	}
	d_min = get_distance(current_position,pos_balise,chemin);
	position_t next_eoa = pos_balise;
	if(train2_on_tracks){
		d = get_distance(current_position,pos_t2,chemin) < d_min;
		if(d<d_min){
			d_min = d;
			next_eoa = train2;
		}
	}
	if(train3_on_tracks){
		d = get_distance(current_position,pos_t3,chemin) < d_min;
		if(d<d_min){
			d_min = d;
			next_eoa = train3;
		}
	}
	pthread_mutex_unlock(&pos_trains_locks[1]);
	pthread_mutex_unlock(&pos_trains_locks[2]);
	pthread_mutex_unlock(&pos_trains_locks[3]);
	return next_eoa;
} 

    
