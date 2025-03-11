#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/can.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include "position.h"
#include "Bibli_automatique.h"

/** 
 *	@brief Calcule la distance entre deux positions sur un même chemin. Une position est définie par sa balise et sa position relative à cette balise.
 *	@param bal1 numero de la balise de départ.
 *	@param bal1 numero de la balise d'arrivée.
 *	@param pos_r_1 position relative (en cm) entre la pos de départ et la balise 1.
 *	@param pos_r_2 position relative (en cm) entre la pos d'arrivée et la balise 2.
 *	@return La distance en cm entre les deux positions.
**/
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

/** 
 *	@brief Donne la consigne en vitesse à partir de l'etat actuel du train.
 *	@param state état du train, composée de [distance à la destination,vitesse_actuelle] en [cm,cm/s].
 *	@return La commande en vitesse (en cm/s) nécessaire pour arriver à destination sans dépassement et en respectant les courbes de freinage (TODO).
**/
int get_new_speed(int state[2]){ //TODO : Implémenter meilleure automatique + courbe de freinage
    int G = 2;
    int K[2] = {2,2};
    int new_speed = (int) state[1]/10;

    return new_speed;
}

/** 
 *	@brief Envoie une commande sur le bus CAN de l'EVC pour commander la vitesse.
 *	@param v vitesse demandée en cm/s.
 *	@return valeur d'erreur (1 si erreur, sinon 0).
**/
int mc_consigneVitesse(int v){
	   // Créer un socket CAN
    int sock;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;

    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        // perror("socket");
        return 1;
    }

    // Spécifier l'interface can0
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Associer le socket à l'interface can0
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Préparer le message CAN (ID 0x17 et données 0x0001)
    frame.can_id = 0x17;  // Identifiant du message CAN
    frame.can_dlc = 2;    // Taille des données (2 octets)
	if(v>0){
		frame.data[0] = v;
		frame.data[1] = 0x01;	
	}
	else{
		frame.data[0] = -v;
		frame.data[1] = 0x00;
	}

    // Envoyer le message
    if (write(sock, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("write");
        return 1;
    }	
    close(sock);
    return 0;
}

/** 
 *	@brief Initie un socket sur le bus can0.
 *	@return valeur d'erreur (1 si erreur, sinon 0).
**/
int init_socket(){
	struct sockaddr_can addr;
	struct ifreq ifr;
	int sock;
	// Ouvrir le socket CAN
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket CAN");
        return -1;
    }

    // Récupérer l'interface can0
    strncpy(ifr.ifr_name, "can0", IFNAMSIZ);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        return -1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Lier le socket à l'interface CAN
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
	return sock;
}

/** 
 *	@brief Récupère la valeur de la vitesse dans le frame du can_frame.
 *	@param frame frame récuperée sur le can0 de l'EVC avec read(sock, &frame, sizeof(struct can_frame).
 *	@return vitesse en cm/s.
**/
int get_speed(struct can_frame frame){
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

/** 
 *	@brief Récupère la valeur de la position dans le frame du can_frame.
 *	@param frame frame récuperée sur le can0 de l'EVC avec read(sock, &frame, sizeof(struct can_frame).
 *	@return position relative à la dernière balise detectée (calculée par endométrie).
**/
float get_relative_pos(struct can_frame frame){
	uint16_t wd = ((uint16_t)frame.data[3] << 8) | frame.data[2];
	float pos = 10*wd*PAS_ROUE_CODEUSE;
	return pos;
}

/** 
 *	@brief Initie l'automatique du train en le faisant avancer jusqu'a la première balise (afin qu'il connaisse sa position initiale).
 *	@return numéro de la première balise rencontrée.
**/
int init_train() {
    struct can_frame frame;
	int sock = init_socket();
	int last_balise = 0;
	
    if(sock == -1){
		return 0;
	}
	mc_consigneVitesse(speed_decouverte);
    while (last_balise == 0) {
        // Lire un message CAN
        if (read(sock, &frame, sizeof(struct can_frame)) < 0) {
            perror("read error");
            return 0;
        }
		if (frame.can_id == 0x30) { //On passe sur une nouvelle balise
			last_balise = frame.data[5];
        }
	}
	close(sock);
	return last_balise;
}


/** 
 *	@brief Fonction qui tourne en permanence sur l'EVC, afin de faire avancer le train jusqu'a la destion pointée par pos_destination. Actualise aussi la position actuelle en permanence.
 *  @param pos_destination pointeur vers la position à rejoindre.
 *  @param pos_current pointeur vers la variable de stockage de la position actuelle.
 *  @param max_speed pointeur vers la vitesse maximale autorisée (en cm/s).
 *  @param chemin liste des balises dont l'ordre définie la boucle que dois faire le train.
 *  @param taille_chemin len(chemin).
 *  @param lock_dest mutex pour l'écriture/lecture de position_destination
 *  @param lock_current mutex pour l'écriture/lecture de pos_current
 *	@return valeur d'erreur (1 si erreur, sinon 0
**/
int boucle_get_to_pos(position_t *pos_destination, position_t *pos_current, int *max_speed, const int* chemin, const int taille_chemin, pthread_mutex_t lock_dest, pthread_mutex_t lock_current) {
    struct can_frame frame;
	clock_t start, end;
    double elapsed_time;
	float integration_erreur = 0;
	float erreur;
	int sock = init_socket();
	int current_speed;
	float dist;
	
    if(sock == -1){
		return 0;
	}
	start = clock();
    while (1) {
        // Lire un message CAN
        if (read(sock, &frame, sizeof(struct can_frame)) < 0) {
            perror("read error");
            return 1;
        }
		if (frame.can_id == 0x30) { //On passe sur une nouvelle balise
			pthread_mutex_lock(&lock_current);
			pos_current->bal = frame.data[5];
			pthread_mutex_unlock(&lock_current);
        }
        if (frame.can_id == 0x02F){
			end = clock();
			// Calcul du temps écoulé en secondes
			elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
			start = end;
			// Actualisation des variables
			pthread_mutex_lock(&lock_current);
			pos_current->pos_r = get_relative_pos(frame);
			pthread_mutex_unlock(&lock_current);
			current_speed = get_speed(frame);
			pthread_mutex_lock(&lock_dest);
			dist = get_distance(*pos_current,*pos_destination,chemin,taille_chemin);
			pthread_mutex_unlock(&lock_dest);
			// Commande de la vitesse
			int state[2] = {current_speed, (int) dist};
			int newSpeed = get_new_speed(state);
			printf("Balise %d -- Pos %f -- Speed %d -- Distance %f -- D_Speed %d \n",pos_current->bal,pos_current->pos_r,current_speed,dist,newSpeed);
			fflush(stdout);
			if(newSpeed>*max_speed) newSpeed =*max_speed;
			if(newSpeed<=min_speed) newSpeed = 0;
			mc_consigneVitesse(newSpeed);
		}
	}
	close(sock);
    return 0;
}

// Initie le train en mode test si le premier argument est auto. Sinon arrete le train (utile pour arret d'urgence)
int main(int argc , char** argv) {
	int last_balise = init_train();
	// Mutex pour protéger l'accès à la variable
	pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
	position_t* pos_destination = malloc(sizeof(position_t));  // Allocation dynamique
	pos_destination->bal = 3;
	pos_destination->pos_r = 20.0;
	position_t* pos_current = malloc(sizeof(position_t));  // Allocation dynamique
	pos_current->bal = last_balise;
	pos_current->pos_r = 0.0;
	int max_speed = 30;
	printf("go \n");
	boucle_get_to_pos(pos_destination,pos_current,&max_speed,chemin1,taille_chemin1,lock1,lock2);
}