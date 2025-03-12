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

void ask_resources(int * next_bal_index, int no_train, position_t * pos_trains);
void free_resources(int * next_bal_index, int no_train, position_t * pos_trains);
position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins);

#define DEBUG_EOA 0
#define DEBUG_RES 0


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
			free_resources(&next_bal_index_lib[recv_message.train_id - 1],recv_message.train_id - 1, pos_trains);
			pthread_mutex_unlock(&pos_trains_locks[recv_message.train_id - 1]);


			send_message->code = 201;
			send_message->data[0] = NULL;
			break;

		case 102:
			printf("RBC [%d] - Demande d'autorisation de mouvement reçue\n", recv_message.train_id);
			ask_resources(&next_bal_index_req[recv_message.train_id - 1],recv_message.train_id - 1, pos_trains);

			printf("Ressource accordée : %d", next_bal_index_req[recv_message.train_id - 1]);

			position_t EOA = next_eoa(recv_message.train_id,pos_trains,next_bal_index_req[recv_message.train_id - 1], chemins, tailles_chemins);

			send_message->code = 202;
			snprintf(send_message->data[0], 10, "%d", EOA.bal);
			snprintf(send_message->data[1], 10, "%.2f", EOA.pos_r);

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
	if (DEBUG_RES){
		printf("\n\nDans demande ressource\nPosition bal: %d, pos: %f\n", pos_trains[no_train].bal, pos_trains[no_train].pos_r);
		printf("Indice de la prochaine ressource : %d \nRessource à verrouiller : %d\n", *next_bal_index, L_mask_req[no_train][*next_bal_index]);
		printf("Balise actuelle: %d, balise à attendre pour demande: %d\n",pos_trains[no_train].bal, L_res_req[no_train][(*next_bal_index - 1)%tailles_chemins[no_train]]);
	}

	// Si la position du train correspond à la position de demande de la prochaine ressource
	// On veut vérifier : bal actuelle = bal de ressource - 1
	if (DEBUG_RES){
		printf("Taille chemin %d: %d\n", no_train, tailles_chemins[no_train]);
	}
	int resource_to_lock = 0;
	for (int i=0;i<tailles_chemins[no_train];i++){
		if (DEBUG_RES){
			printf("Dans loop\n");
		}
		// donc bal actuelle = bal d'indice i - i
		int condition_1 = (pos_trains[no_train].bal == chemins[no_train][(i-1)%tailles_chemins[no_train]]);
		// ou bal actuelle = bal d'indice i
		int condition_1bis = (pos_trains[no_train].bal == chemins[no_train][i]);
		// ET bal de ressource = bal d'indice i
		int condition_2 = (L_res_req[no_train][*next_bal_index] == chemins[no_train][i]);
		if (DEBUG_RES){
			printf("Balise actuelle %d\nBalise d'indice i - 1 %d\nBalise de ressource %d\nBalise d'indice i %d\n", pos_trains[no_train].bal, chemins[no_train][(i-1)%tailles_chemins[no_train]], L_res_req[no_train][*next_bal_index], chemins[no_train][i]);
		}

		if ((condition_1 || condition_1bis) && condition_2){
			resource_to_lock = 1;
		}
	}
	if (resource_to_lock && (pos_trains[no_train].pos_r >= 0)){
		if (DEBUG_RES){
			printf("Ressources avant lock: %d\n", resources);
		}
		lock_ressources(L_mask_req[no_train][*next_bal_index]);
		if (DEBUG_RES){
			printf("Ressources après lock: %d\n", resources);
		}

	if (DEBUG_RES){
		printf("Test condition :\n Indice actuel %d\n Limite %ld\n", *next_bal_index, sizeof(L_res_req[no_train])/sizeof(int));
	}

		// Si on est au bout du chemin, la prochaine balise est à nouveau la balise 0.
		// Sinon, c'est la prochaine dans le chemin
		*next_bal_index = (*next_bal_index + 1)%tailles_chemins[no_train];
		}
	
	if (DEBUG_RES){
		printf("Indice de la prochaine ressource : %d \nProchaine ressource à verrouiller : %d\n", *next_bal_index, L_mask_req[no_train][*next_bal_index]);

		printf("Sortie demande ressource\n\n");
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
	if (DEBUG_EOA){
	printf("Entrée dans next EOA\nNo train: %d\n Pos train actuel: %d, %f\nProchaine balise avant ressource: %d\n", num_train, pos_trains[num_train].bal, pos_trains[num_train].pos_r, next_balise_avant_ressource);
	}
    int i = 0; 
    position_t current_position = pos_trains[num_train];
    const int * chemin = chemins[num_train];
    int len_chemin = len_chemins[num_train];
    // On se place sur la balise de la position actuelle
	while(chemin[i] != current_position.bal){
		i++;
	}
    int num_balise_check = i;
		if (DEBUG_EOA){
			printf("Indice balise actuelle : %d\n", num_balise_check);
			printf("Balise actuelle supposée : %d\n", chemin[i]);
			printf("Balise actuelle réelle : %d\n", pos_trains[num_train].bal);
		}

    // Cas spécial où la balise actuelle est la même qu'un des objectif (et devant notre position)
    if(next_balise_avant_ressource == chemin[num_balise_check] && current_position.pos_r <= 0) return((position_t) {next_balise_avant_ressource, 0.0});
    if(pos_trains[(num_train+1)%3].bal == chemin[num_balise_check] && pos_trains[(num_train+1)%3].pos_r >= current_position.pos_r) return(pos_trains[(num_train+1)%3]);
    if(pos_trains[(num_train+2)%3].bal == chemin[num_balise_check] && pos_trains[(num_train+2)%3].pos_r >= current_position.pos_r) return(pos_trains[(num_train+2)%3]);
    // Cas général, recherche du premier objectif trouvé
    while(1)
    {
        num_balise_check = (num_balise_check+1)%len_chemin;
		if (DEBUG_EOA){
			printf("Indice prochaine balise : %d\n", num_balise_check);
		}
        int objectif_sur_balise[3] = {0,0,0};
        float list_pos_r[3] = {0.0,0.0,0.0};
        if(next_balise_avant_ressource == chemin[num_balise_check]){
			if (DEBUG_EOA){
				("Prochaine balise bloquée par ressource : %d", next_balise_avant_ressource);
			}
            objectif_sur_balise[0] = 1;
            list_pos_r[0] = 0.0;
        }
        if(pos_trains[(num_train+1)%3].bal == chemin[num_balise_check]){
			if (DEBUG_EOA){
				("Prochaine balise bloquée par train+1 : %d", next_balise_avant_ressource);
			}
            objectif_sur_balise[1] = 1;
            list_pos_r[1] = pos_trains[(num_train+1)%3].pos_r;
        }
        if(pos_trains[(num_train+2)%3].bal == chemin[num_balise_check]){
			if (DEBUG_EOA){
				("Prochaine balise bloquée par train+2 : %d", next_balise_avant_ressource);
			}
            objectif_sur_balise[2] = 1;
            list_pos_r[2] = pos_trains[(num_train+2)%3].pos_r;
        }
        // Calcul du minimum des pos_r parmis ceux sur la balise_check (si il y en as)
        if(objectif_sur_balise[0] || objectif_sur_balise[1] || objectif_sur_balise[2]){
            float min_pos_r = 1E9;
            for(int j = 0; j<=2; j++){
                if(objectif_sur_balise[j] && list_pos_r[j] <= min_pos_r){
                    min_pos_r = list_pos_r[j];
                }
            }
            return((position_t) {chemin[num_balise_check], min_pos_r});
        }
    }
}

// int main(){
//     int num_train = 0;
//     const int chemin3[] = {22,23,24,25,250,26,251,27,28,29,11,12,13,7,8,9};
//     const int chemin2[] = {11,251,26,250,25,20,21,17};
//     const int chemin1[] = {7,8,9,1,2,3,12,13};
//     const int * chemins[3] = {chemin1, chemin2, chemin3};
//     const int len_chemins[3] = {sizeof(chemin1)/sizeof(int), sizeof(chemin2)/sizeof(int), sizeof(chemin3)/sizeof(int)};
//     position_t pos1 = {9,20.0};
//     position_t pos2 = {2,60.0};
//     position_t pos3 = {2,50.0};
//     position_t pos_trains[3] = {pos1,pos2,pos3};
//     int next_balise_avant_ressource = 2;
//     position_t EOA = next_eoa(num_train,pos_trains,next_balise_avant_ressource, chemins, len_chemins);
//     printf("balise EOA : %d , Position relative : %f \n" , EOA.bal, EOA.pos_r);
// }

