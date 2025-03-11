#ifndef _UNIRAIL_AUTOMATIQUE_H
	#define _UNIRAIL_AUTOMATIQUE_H
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

	#define PAS_ROUE_CODEUSE 0.016944 // en cm

	typedef struct {
		position_t * destination;
		pthread_mutex_t * destination_lock;
		int * max_speed;
		int chemin_id;
	} consigne_t;

	float get_distance(position_t pos_current, position_t pos_destination, const int* chemin, int taille_chemin);
	int compute_new_speed(int state[2]);
	int mc_consigneVitesse(int can_socket, int v);
	int read_speed_from_frame(struct can_frame frame);
	float read_relative_pos_from_frame(struct can_frame frame);
	void * boucle_automatique(position_t * position, pthread_mutex_t * position_lock, consigne_t consigne, const int can_socket);
#endif