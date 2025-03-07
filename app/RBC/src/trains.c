#include "../include/trains.h"

position_t pos_trains[NB_TRAINS]; 
pthread_mutex_t pos_trains_locks[NB_TRAINS];

void init_trains() {
	for (int i = 0; i < NB_TRAINS; i++) {
		pos_trains[i].bal = 0;
		pos_trains[i].pos_r = 0.0;
		pthread_mutex_init(&pos_trains_locks[i], NULL);
	}
}