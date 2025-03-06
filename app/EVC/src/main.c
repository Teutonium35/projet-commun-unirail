#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
                                    { perror(msg); \
                                        exit(EXIT_FAILURE); }

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "../include/position.h"

position_t pos = {0, 0.0};
pthread_mutex_t pos_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    if (argc != 4) printf("EVC | Utilisation : ./rbc <id train> <adresse_serveur> <port serveur> \n");
    else {
        
		pthread_t posreport_tid;
		client_udp_init_t client;

		int train_id = atoi(argv[1]);
		
        printf("EVC - Initialisation\n");

		setup_udp_client(&client, argv[2], atoi(argv[3]));
		
		report_position_args_t rpa = {client, train_id, &pos, &pos_mutex};

		pthread_create(&posreport_tid, NULL, report_position, &rpa);

		pthread_join(posreport_tid, NULL);

    exit(EXIT_SUCCESS);
    }
}