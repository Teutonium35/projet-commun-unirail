#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/can.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include "position.h"
#include "Bibli_automatique.h"

#define PAS_ROUE_CODEUSE 0.016944 // en cm

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
int get_new_speed(int state[2]){ //TODO : Implémenter meilleure automatique
    int G = 2;
    int K[2] = {2,2};
    int new_speed = (int) state[1]/10;

    return new_speed;
}

//Envoie sur le bus can de l'EVC une commande de vitesse pour faire avancer le train à la vitesse v
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

float get_relative_pos(struct can_frame frame){
	uint16_t wd = ((uint16_t)frame.data[3] << 8) | frame.data[2];
	float pos = 10*wd*PAS_ROUE_CODEUSE;
	return pos;
}

int init_train() {
    struct can_frame frame;
	int sock = init_socket();
	int last_balise = 0;
	
    if(sock == -1){
		return 0;
	}
	mc_consigneVitesse(20);
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
			pos_current->bal = frame.data[5];
        }
        if (frame.can_id == 0x02F){
			end = clock();
			// Calcul du temps écoulé en secondes
			elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
			start = end;
			pos_current->pos_r = get_relative_pos(frame);
			current_speed = get_speed(frame);
			pthread_mutex_lock(&lock_dest);
			dist = get_distance(*pos_current,*pos_destination,chemin,taille_chemin);
			pthread_mutex_unlock(&lock_dest);
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