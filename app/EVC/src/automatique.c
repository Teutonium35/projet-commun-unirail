#include "automatique.h"
#include "../../utility/include/can.h"

const int B1[] = {1,2,3,3,3,4,5,6,7,8,9,10,11,11,12,13,14,15,15,16,17,12,18,19,20,21,21,22,23,24,25,250,26,251,27,28,29,13,9,9,16,16,16,9,25,11};
const int B2[] = {2,3,4,12,26,5,6,7,8,9,1,11,12,26,13,14,15,16,21,10,11,18,19,20,21,17,22,23,24,25,250,26,251,27,28,29,11,7,17,22,22,1,17,10,20,251};
const float D[] = {1671.0, 1898.0, 2264.0, 2673.0, 3895.0, 1617.0, 1612.0, 2134.0, 1835.0, 1628.0, 2785.0, 2769.0, 2419.0, 3500.0, 2084.0, 2104.0, 1489.0, 1975.0, 2636.0, 3754.0, 2968.0, 2056.0, 1447.0, 2261.0, 2494.0, 2608.0, 3272.0, 1664.0, 1716.0, 1703.0, 643.0, 1165.0, 1025.0, 2015.0, 1580.0, 1903.0, 3008.0, 2628.0, 3416.0, 4089.0, 4242.0, 2943.0, 3570.0, 3620.0, 2543.0 ,2534.8};

const int chemin3[] = {22,23,24,25,250,26,251,27,28,29,11,12,13,7,8,9};
const int chemin2[] = {11,251,26,250,25,20,21,17};
const int chemin1[] = {7,8,9,1,2,3,12,13};

const int * chemins[3] = {chemin1, chemin2, chemin3};
const int tailles_chemins[3] = {sizeof(chemin1)/sizeof(int), sizeof(chemin2)/sizeof(int), sizeof(chemin3)/sizeof(int)};

const int min_speed = 2;

// Calcule la distance entre deux positions sur un même chemin. Les positions sont définies par leurs numéros de balises (int bal1 et bal2) et leurs postions relatives à cette balise (pos_r_1 et pos_r_2)
float get_distance(position_t pos_current, position_t pos_destination, const int* chemin, int taille_chemin){
	float pos_r_1 = pos_current.pos_r;
	float pos_r_2 = pos_destination.pos_r;
	int bal1 = pos_current.bal;
	int bal2 = pos_destination.bal;
	float distance = pos_r_2 - pos_r_1; //Il est clair qu'on doit ajouter la pos relative à la bal2. De plus dans la suite on calcule la distance entière entre bal1 et la bal suivante. Donc il faut soustraire bal2
	int i = 0; 
	while(chemin[i] != bal1){ //On se place sur la bal1
		i++;
	}
	int do_first_loop = 0; // Sert à forcer le processus de faire la premiere loop du while dans certains cas. Cf if suivante
	if(bal1  == bal2 && (pos_r_1 - pos_r_2 >= 50)){ //On verrifie si on a dépassé la pos_2, avec une petite marge d'erreur.
		do_first_loop = 1; //Dans ce cas, on est dans le même canton mais devant notre pos_2. Donc on doit faire un tour entier du circuit. Cela se traduit pas une entrée forcée dans la boucle while lors de la premiere boucle
	}
	printf("b");
	while(chemin[i] != bal2 || do_first_loop == 1){ //On continue notre chemin et sommons les distances tant qu'on est pas arrivé à bal2
		int j = 0;
		do_first_loop = 0;
		int next_index = (i==(taille_chemin-1))? 0 : (i+1);
		while((B1[j] != chemin[i] || B2[j] != chemin[next_index]) && (B1[j] != chemin[next_index] || B2[j] != chemin[i])){ //On trouve l'arc qui va de notre balise jusqu'a la prochaine balise
			j++;
		}
		distance += D[j]; //On sommes les distances de balises en balises
		i = next_index;
	}
	return distance;
}

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
			dist = get_distance(pos_current, *consigne.destination, chemins[consigne.chemin_id], tailles_chemins[consigne.chemin_id]);
			pthread_mutex_unlock(consigne.destination_lock);

			int state[2] = {current_speed, (int) dist};
			int newSpeed = compute_new_speed(state);
			printf("Balise %d -- Pos %f -- Speed %d -- Distance %f -- D_Speed %d \n",pos_current.bal,pos_current.pos_r,current_speed,dist,newSpeed);
			fflush(stdout);

			if(newSpeed > *consigne.max_speed) newSpeed = *consigne.max_speed;
			if(newSpeed <= min_speed) newSpeed = 0;
			mc_consigneVitesse(can_socket, newSpeed);
		}
	}
    
}