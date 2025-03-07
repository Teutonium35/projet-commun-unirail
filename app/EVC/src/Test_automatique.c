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




#define PAS_ROUE_CODEUSE 0.016944 // en cm

const int B1[] = {1,2,3,3,3,4,5,6,7,8,9,10,11,11,12,13,14,15,15,16,17,12,18,19,20,21,21,22,23,24,25,250,26,251,27,28,29,13,9,9,16,16,16,9};
const int B2[] = {2,3,4,12,26,5,6,7,8,9,1,11,12,26,13,14,15,16,21,10,11,18,19,20,21,17,22,23,24,25,250,26,251,27,28,29,11,7,17,22,22,1,17,10};
const float D[] = {1671.0, 1898.0, 2264.0, 2673.0, 3895.0, 1617.0, 1612.0, 2134.0, 1835.0, 1628.0, 2785.0, 2769.0, 2419.0, 3500.0, 2084.0, 2104.0, 1489.0, 1975.0, 2636.0, 3754.0, 2968.0, 2056.0, 1447.0, 2261.0, 2494.0, 2608.0, 3272.0, 1664.0, 1716.0, 1703.0, 643.0, 1165.0, 1025.0, 2015.0, 1580.0, 1903.0, 3008.0, 2628.0, 3416.0, 4089.0, 4242.0, 2943.0, 3570.0, 3620.0};

const int chemin[] = {22,23,24,25,250,26,251,27,28,29,11,12,13,7,8,9};
const int n_balises = sizeof(chemin)/sizeof(int);

const int balise_objectif = 22;
const float objectif_roue = 500.0;


// Calcule la distance entre deux positions sur un m�me chemin. Les positions sont d�finies par leurs num�ros de balises (int bal1 et bal2) et leurs postions relatives � cette balise (pos_r_1 et pos_r_2)
float get_distance(int bal1, int bal2, float pos_r_1, float pos_r_2){
	float distance = pos_r_2 - pos_r_1; //Il est clair qu'on doit ajouter la pos relative � la bal2. De plus dans la suite on calcule la distance enti�re entre bal1 et la bal suivante. Donc il faut soustraire bal2
	int i = 0; 
	while(chemin[i] != bal1){ //On se place sur la bal1
		i++;
	}
	int do_first_loop = 0; // Sert � forcer le processus de faire la premiere loop du while dans certains cas. Cf if suivante
	if(bal1  == bal2 && (pos_r_1 - pos_r_2 >= 50)){ //On verrifie si on a d�pass� la pos_2, avec une petite marge d'erreur.
		do_first_loop = 1; //Dans ce cas, on est dans le m�me canton mais devant notre pos_2. Donc on doit faire un tour entier du circuit. Cela se traduit pas une entr�e forc�e dans la boucle while lors de la premiere boucle
	}
	while(chemin[i] != bal2 || do_first_loop == 1){ //On continue notre chemin et sommons les distances tant qu'on est pas arriv� � bal2
		int j = 0;
		do_first_loop = 0;
		int next_index = (i==(n_balises-1))? 0 : (i+1);
		while((B1[j] != chemin[i] || B2[j] != chemin[next_index]) && (B1[j] != chemin[next_index] || B2[j] != chemin[i])){ //On trouve l'arc qui va de notre balise jusqu'a la prochaine balise
			j++;
		}
		distance += D[j]; //On sommes les distances de balises en balises
		i = next_index;
	}
	return distance;
}

//Donne la consigne en vitesse � partir de state qui est [vitesse,distance_a_objectif]
int get_new_speed(int state[2]){ //TODO : Impl�menter meilleure automatique
    int G = 2;
    int K[2] = {2,2};
    int new_speed = (int) state[1]/10;

    return new_speed;
}

//Envoie sur le bus can de l'EVC une commande de vitesse pour faire avancer le train � la vitesse v
int mc_consigneVitesse(int v){
	   // Cr�er un socket CAN
    int sock;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;

    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        // perror("socket");
        return 1;
    }

    // Sp�cifier l'interface can0
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Associer le socket � l'interface can0
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Pr�parer le message CAN (ID 0x17 et donn�es 0x0001)
    frame.can_id = 0x17;  // Identifiant du message CAN
    frame.can_dlc = 2;    // Taille des donn�es (2 octets)
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

    // R�cup�rer l'interface can0
    strncpy(ifr.ifr_name, "can0", IFNAMSIZ);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        return -1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Lier le socket � l'interface CAN
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

int full_auto() {
    struct can_frame frame;
	int last_balise = 0;
	clock_t start, end;
    double elapsed_time;
	float integration_erreur = 0;
	float erreur;
	int sock = init_socket();
	int current_speed;
	float dist;
	float current_r_pos;
	
    if(sock == -1){
		return 0;
	}
	start = clock();
	mc_consigneVitesse(20);
    while (1) {
        // Lire un message CAN
        if (read(sock, &frame, sizeof(struct can_frame)) < 0) {
            perror("read");
            return 1;
        }
		if (frame.can_id == 0x30) { //On passe sur une nouvelle balise
			last_balise = frame.data[5];
            printf("\n");
        }
        if (frame.can_id == 0x02F && last_balise != 0){ //Les mesures et la commande ne se font que lorsqu'on sait o� on est. Lorsque la premiere balise est rencontr�e
			end = clock();
			// Calcul du temps �coul� en secondes
			elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
			start = end;	
			current_r_pos = get_relative_pos(frame);
			current_speed = get_speed(frame);
			dist = get_distance(last_balise,7,current_r_pos,70.0);
			int state[2] = {current_speed, (int) dist};
			int newSpeed = get_new_speed(state);
			printf("Balise %d -- Pos %f -- Speed %d -- Distance %f -- D_Speed %d \n",last_balise,current_r_pos,current_speed,dist,newSpeed);
			fflush(stdout);
			if(newSpeed>40) newSpeed =40;
			if(newSpeed<3) newSpeed = 0;
			mc_consigneVitesse(newSpeed);
		}
			
			//erreur = (1000-pos);	
			//integration_erreur += erreur*elapsed_time;
			//printf("P : %f - I : %f \n", erreur*0.03,integration_erreur*0.03);
			//printf("commande vitesse : %d \n",(int)(erreur*0.06 + integration_erreur*0.02));
			//mc_consigneVitesse((int)(erreur*0.03 + integration_erreur*0.03));
	}
	close(sock);
    return 0;
}