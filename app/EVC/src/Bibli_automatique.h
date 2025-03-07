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

const int B1[] = {1,2,3,3,3,4,5,6,7,8,9,10,11,11,12,13,14,15,15,16,17,12,18,19,20,21,21,22,23,24,25,250,26,251,27,28,29,13,9,9,16,16,16,9,25,11};
const int B2[] = {2,3,4,12,26,5,6,7,8,9,1,11,12,26,13,14,15,16,21,10,11,18,19,20,21,17,22,23,24,25,250,26,251,27,28,29,11,7,17,22,22,1,17,10,20,251};
const float D[] = {1671.0, 1898.0, 2264.0, 2673.0, 3895.0, 1617.0, 1612.0, 2134.0, 1835.0, 1628.0, 2785.0, 2769.0, 2419.0, 3500.0, 2084.0, 2104.0, 1489.0, 1975.0, 2636.0, 3754.0, 2968.0, 2056.0, 1447.0, 2261.0, 2494.0, 2608.0, 3272.0, 1664.0, 1716.0, 1703.0, 643.0, 1165.0, 1025.0, 2015.0, 1580.0, 1903.0, 3008.0, 2628.0, 3416.0, 4089.0, 4242.0, 2943.0, 3570.0, 3620.0, 2543.0 ,2534.8};

const int chemin3[] = {22,23,24,25,250,26,251,27,28,29,11,12,13,7,8,9};
const int chemin2[] = {11,251,26,250,25,20,21,17};
const int chemin1[] = {7,8,9,1,2,3,12,13};

const int taille_chemin3 = sizeof(chemin3)/sizeof(int);
const int taille_chemin2 = sizeof(chemin2)/sizeof(int);
const int taille_chemin1 = sizeof(chemin1)/sizeof(int);

const int min_speed = 2;

float get_distance(position_t pos_current, position_t pos_destination, const int* chemin, int taille_chemin);
int get_new_speed(int state[2]);
int mc_consigneVitesse(int v);
int init_socket();
int get_speed(struct can_frame frame);
float get_relative_pos(struct can_frame frame);
int boucle_get_to_pos(position_t *pos_destination, position_t *pos_current, int *max_speed, const int* chemin, const int taille_chemin, pthread_mutex_t lock_dest, pthread_mutex_t lock_current);