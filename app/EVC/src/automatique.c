#include "automatique.h"
#include "../../utility/include/can.h"
#include "../../utility/include/debug.h"

int min_speed = 2;

//Donne la consigne en vitesse à partir de state qui est [vitesse,distance_a_objectif]
int compute_new_speed(int state[2]){ //TODO : Implémenter meilleure automatique
    int G = 2;
    int K[2] = {2,2};
    int new_speed = (int) state[1]/10;

    return new_speed;
}

//Envoie sur le bus can de l'EVC une commande de vitesse pour faire avancer le train à la vitesse v
int mc_consigneVitesse(int can_socket, int speed){
    struct can_frame frame;

	int data[8];
	data[0] = speed > 0 ? speed : -speed;
	data[1] = speed > 0 ? 0x01 : 0x00;

	return send_can_data(can_socket, 0x17, 2, data);
}

int read_speed_from_frame(struct can_frame frame){
	int speed;
	if (frame.data[1] >127)
	{
		frame.data[1] = ~frame.data[1];
		speed = -(frame.data[1]+1);
	}
	else
		speed = frame.data[1];
	return speed;
}

float read_relative_pos_from_frame(struct can_frame frame){
	uint16_t wd = ((uint16_t)frame.data[3] << 8) | frame.data[2];
	float pos = 10*wd*PAS_ROUE_CODEUSE;
	return pos;
}

int get_limit_speed(position_t pos_current, const int chemin_id){
	const int* chemin = chemins[chemin_id - 1];
	const int taille_chemin = tailles_chemins[chemin_id - 1];
	int bal1 = pos_current.bal;
	int i = 0; 
	float a;
	while(chemin[i] != bal1){ //On se place sur la bal1
		i++;
	}
	int v1 = v_limits[chemin_id - 1][i];
	int v2 = v_limits[chemin_id - 1][(i+1)%taille_chemin];
	int j = 0;
	while((B1[j] != chemin[i] || B2[j] != chemin[(i+1)%taille_chemin]) && (B1[j] != chemin[(i+1)%taille_chemin] || B2[j] != chemin[i])){ //On trouve l'arc qui va de notre balise jusqu'a la prochaine balise
		j++;
	}
	if(v1>=v2) a = max_deacceleration;
	else() a = max_acceleration;
	float pos_r_debut_acceleration = D[j] - ((v2^2 - v1^2))/(2*a);
	printf("distance freinage/acceleration : %s \n" , ((v2^2 - v1^2))/(2*a))
	if(pos_current.pos_r < pos_r_debut_acceleration) (return v1);
	if(pos_current.pos_r > D[j]) (return v2);
	return (sqrtf(v1^2 + 2* a * (pos_current.pos_r - pos_r_debut_acceleration)));
}


int init_train(int can_socket) {
    struct can_frame frame;
	int last_balise = 0;

	mc_consigneVitesse(can_socket, 20);
    while (last_balise == 0) {
        // Lire un message CAN
        if (read(can_socket, &frame, sizeof(struct can_frame)) < 0) {
            perror("read error");
            return 0;
        }
		if (frame.can_id == 0x30) { //On passe sur une nouvelle balise
			last_balise = frame.data[5];
        }
	}
	return last_balise;
}

void * boucle_automatique(position_t * position, pthread_mutex_t * position_lock, consigne_t consigne, const int can_socket) {
    struct can_frame frame;
	clock_t start, end;
	position_t pos_current;
    double elapsed_time;
	float integration_erreur = 0;
	float erreur;
	int current_speed;
	float dist;
	int speed_limit = 0;

	int last_balise = init_train(can_socket);

	pos_current.bal = last_balise;
	pos_current.pos_r = 0.0;
	
	start = clock();
    while (1) {
		pthread_mutex_lock(position_lock);
		position->bal = pos_current.bal;
		position->pos_r = pos_current.pos_r;
		pthread_mutex_unlock(position_lock);

        // Lire un message CAN
        if (read(can_socket, &frame, sizeof(struct can_frame)) < 0) {
			fprintf(stderr, "Failed to read data from CAN\n");
            continue;
        }
		if (frame.can_id == 0x30) { //On passe sur une nouvelle balise
			pos_current.bal = frame.data[5];
        }
        if (frame.can_id == 0x02F){
			end = clock();
			// Calcul du temps écoulé en secondes
			elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
			start = end;

			pos_current.pos_r = read_relative_pos_from_frame(frame);
			current_speed = read_speed_from_frame(frame);

			pthread_mutex_lock(consigne.destination_lock);
			if (consigne.destination->bal < 1) dist = 0;
			else{
				dist = get_distance(pos_current, *consigne.destination, consigne.chemin_id);
				speed_limit = get_limit_vitesse(pos_current,consigne.chemin_id);
			}
			pthread_mutex_unlock(consigne.destination_lock);

			int state[2] = {current_speed, (int) dist};
			int newSpeed = compute_new_speed(state);
			
			DEBUG_PRINT("Balise %d -- Pos %f -- Speed %d -- Distance %f -- D_Speed %d \n",pos_current.bal,pos_current.pos_r,current_speed,dist,newSpeed);
			fflush(stdout);


			if(newSpeed > speed_limit) newSpeed = speed_limit;
			if(newSpeed <= min_speed) newSpeed = 0;
			mc_consigneVitesse(can_socket, newSpeed);
		}
	}
    
}