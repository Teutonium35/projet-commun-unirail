#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "../include/position.h"
#include "../include/ressources.h"
#include "../include/trains.h"



position_t next_eoa(int num_train, position_t *pos_trains, int next_balise_avant_ressource, const int *chemins[3], const int *len_chemins){
    int i = 0; 
    position_t current_position = pos_trains[num_train];
    const int * chemin = chemins[num_train];
    int len_chemin = len_chemins[num_train];
    // On se place sur la balise de la position actuelle
	while(chemin[i] != current_position.bal){
		i++;
	}
    int num_balise_check = i;
    // Cas spécial où la balise actuelle est la même qu'un des objectif (et devant notre position)
    if(next_balise_avant_ressource == chemin[num_balise_check] && current_position.pos_r <= 0) return((position_t) {next_balise_avant_ressource, 0.0});
    if(pos_trains[(num_train+1)%3].bal == chemin[num_balise_check] && pos_trains[(num_train+1)%3].pos_r >= current_position.pos_r) return(pos_trains[(num_train+1)%3]);
    if(pos_trains[(num_train+2)%3].bal == chemin[num_balise_check] && pos_trains[(num_train+2)%3].pos_r >= current_position.pos_r) return(pos_trains[(num_train+2)%3]);
    // Cas général, recherche du premier objectif trouvé
    while(1)
    {
        num_balise_check = (num_balise_check+1)%len_chemin;
        int objectif_sur_balise[3] = {0,0,0};
        float list_pos_r[3] = {0.0,0.0,0.0};
        if(next_balise_avant_ressource == chemin[num_balise_check]){
            objectif_sur_balise[0] = 1;
            list_pos_r[0] = 0.0;
        }
        if(pos_trains[(num_train+1)%3].bal == chemin[num_balise_check]){
            objectif_sur_balise[1] = 1;
            list_pos_r[1] = pos_trains[(num_train+1)%3].pos_r;
        }
        if(pos_trains[(num_train+2)%3].bal == chemin[num_balise_check]){
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

int main(){
    int num_train = 0;
    const int chemin3[] = {22,23,24,25,250,26,251,27,28,29,11,12,13,7,8,9};
    const int chemin2[] = {11,251,26,250,25,20,21,17};
    const int chemin1[] = {7,8,9,1,2,3,12,13};
    const int * chemins[3] = {chemin1, chemin2, chemin3};
    const int len_chemins[3] = {sizeof(chemin1)/sizeof(int), sizeof(chemin2)/sizeof(int), sizeof(chemin3)/sizeof(int)};
    position_t pos1 = {9,20.0};
    position_t pos2 = {2,3.0};
    position_t pos3 = {14,50.0};
    position_t pos_trains[3] = {pos1,pos2,pos3};
    int next_balise_avant_ressource = 2;
    position_t EOA = next_eoa(num_train,pos_trains,next_balise_avant_ressource, chemins, len_chemins);
    printf("balise EOA : %d , Position relative : %f \n" , EOA.bal, EOA.pos_r);
}

