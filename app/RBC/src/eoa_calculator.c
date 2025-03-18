#include <stdio.h>

#include "../include/eoa_calculator.h"
position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins){
	pthread_mutex_lock(&pos_trains_locks[num_train]);
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
    if(next_balise_avant_ressource == chemin[num_balise_check] && current_position.pos_r <= 0){
		pthread_mutex_unlock(&pos_trains_locks[num_train]);
		return((position_t) {next_balise_avant_ressource, 0.0});
	} 
    if(pos_trains[(num_train+1)%3].bal == chemin[num_balise_check] && pos_trains[(num_train+1)%3].pos_r >= current_position.pos_r){
		position_t next_train_position = pos_trains[(num_train+1)%3]; 
		pthread_mutex_unlock(&pos_trains_locks[num_train]);
		return(next_train_position);
	} 
    if(pos_trains[(num_train+2)%3].bal == chemin[num_balise_check] && pos_trains[(num_train+2)%3].pos_r >= current_position.pos_r){
	 	position_t next_train_position = pos_trains[(num_train+2)%3];
		pthread_mutex_unlock(&pos_trains_locks[num_train]);
		return(next_train_position);
	}
    // Cas général, recherche du premier objectif trouvé
    while(1)
    {
        num_balise_check = (num_balise_check+1)%len_chemin;
        int objectif_sur_balise[3] = {0,0,0};
		if (DEBUG_EOA){
			printf("Indice prochaine balise : %d\n", num_balise_check);
			printf("Objectif init 1 %d | 2 %d | 3 %d\n", objectif_sur_balise[0], objectif_sur_balise[1], objectif_sur_balise[2]);
		}
        float list_pos_r[3] = {0.0,0.0,0.0};
        if(next_balise_avant_ressource == chemin[num_balise_check]){
			if (DEBUG_EOA){
				printf("Prochaine balise bloquée par ressource : %d", next_balise_avant_ressource);
			}
            objectif_sur_balise[0] = 1;
            list_pos_r[0] = 0.0;
        }
        if(pos_trains[(num_train+1)%3].bal == chemin[num_balise_check]){
			if (DEBUG_EOA){
				printf("Prochaine balise bloquée par train+1 : %d\n", next_balise_avant_ressource);
				printf("Position train+1 %d\n Balise bloquée %d", pos_trains[(num_train+1)%3].bal, chemin[num_balise_check]);
				printf("Indice train +1 %d, pos train +1 %d, %f\n", (num_train+1)%3, pos_trains[(num_train+1)%3].bal, pos_trains[(num_train+1)%3].pos_r);
			}
            objectif_sur_balise[1] = 1;
            list_pos_r[1] = pos_trains[(num_train+1)%3].pos_r;
        }
        if(pos_trains[(num_train+2)%3].bal == chemin[num_balise_check]){
			if (DEBUG_EOA){
				printf("Prochaine balise bloquée par train+2 : %d\n", next_balise_avant_ressource);
				printf("Position train+2 %d\n Balise bloquée %d", pos_trains[(num_train+2)%3].bal, chemin[num_balise_check]);
				printf("Indice train +2 %d, pos train +2 %d, %f\n", (num_train+2)%3, pos_trains[(num_train+2)%3].bal, pos_trains[(num_train+2)%3].pos_r);
			}
            objectif_sur_balise[2] = 1;
            list_pos_r[2] = pos_trains[(num_train+2)%3].pos_r;
        }
        // Calcul du minimum des pos_r parmis ceux sur la balise_check (si il y en as)
        if(objectif_sur_balise[0] || objectif_sur_balise[1] || objectif_sur_balise[2]){
			if (DEBUG_EOA){
				printf("Objectif trigger 1 %d | 2 %d | 3 %d\n", objectif_sur_balise[0], objectif_sur_balise[1], objectif_sur_balise[2]);
			}
            float min_pos_r = 1E9;
            for(int j = 0; j<=2; j++){
                if(objectif_sur_balise[j] && list_pos_r[j] <= min_pos_r){
                    min_pos_r = list_pos_r[j];
                }
            }
				pthread_mutex_unlock(&pos_trains_locks[num_train]);
            return((position_t) {chemin[num_balise_check], min_pos_r});
        }
    }
}

/*

position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins){
	pthread_mutex_lock(&pos_trains_locks[0]);
	pthread_mutex_lock(&pos_trains_locks[1]);
	pthread_mutex_lock(&pos_trains_locks[2]);
	if (DEBUG_EOA){
	printf("Entrée dans next EOA\nNo train: %d\n Pos train actuel: %d, %f\nProchaine balise avant ressource: %d\n", num_train, pos_trains[num_train].bal, pos_trains[num_train].pos_r, next_balise_avant_ressource);
	}
	float distance_secu = 300.0;
    int i = 0; 
	float d;
	float d_min;
    position_t current_position = pos_trains[num_train];
	position_t pos_balise = (position_t) {next_balise_avant_ressource,0.0};
	position_t pos_t2 = (position_t) {pos_trains[(num_train+1)%3].bal,pos_trains[(num_train+2)%3].pos_r - distance_secu};
	position_t pos_t3 = (position_t) {pos_trains[(num_train+2)%3].bal,pos_trains[(num_train+3)%3].pos_r - distance_secu};
    const int * chemin = chemins[num_train];
    int len_chemin = len_chemins[num_train];
	int train2_on_tracks = 0;
	int train3_on_tracks = 0;
    // On se place sur la balise de la position actuelle
	printf("Before while\n");
	while(chemin[i] != len_chemin){ 
		if(pos_t2.bal == chemin[i]) train2_on_tracks = 1;
		if(pos_t3.bal == chemin[i]) train3_on_tracks = 1;
		i++;
	}
	d_min = get_distance(current_position,pos_balise,num_train + 1);
	position_t next_eoa = pos_balise;
	if(train2_on_tracks){
		d = get_distance(current_position,pos_t2,num_train + 1);
		if(d<d_min){
			d_min = d;
			next_eoa = pos_t2;
		}
	}
	if(train3_on_tracks){
		d = get_distance(current_position,pos_t3,num_train + 1);
		if(d<d_min){
			d_min = d;
			next_eoa = pos_t3;
		}
	}
	pthread_mutex_unlock(&pos_trains_locks[0]);
	pthread_mutex_unlock(&pos_trains_locks[1]);
	pthread_mutex_unlock(&pos_trains_locks[2]);
	return next_eoa;
} 
*/