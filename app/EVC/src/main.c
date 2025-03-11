#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
                                    { perror(msg); \
                                        exit(EXIT_FAILURE); }

#define _GNU_SOURCE

#define DEBUG 0

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

#include "../include/position.h"
#include "../include/automatique.h"
#include "../include/eoa_handler.h"
#include "../../utility/include/can.h"

position_t pos = {0, 0.0};
position_t destination = {3, 20.0};
position_t eoa = {3, 20.0};

pthread_mutex_t pos_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dest_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t eoa_mutex = PTHREAD_MUTEX_INITIALIZER;

int max_speed = 30;
int can_socket;

void install_signal_deroute(int numSig, void (*pfct)(int));
void stop_train();

int main(int argc, char *argv[]) {
    if (argc != 4) printf("EVC | Utilisation : ./rbc <id train> <adresse_serveur> <port serveur> \n");
    else {
        
		pthread_t posreport_tid, eoa_tid;
		client_udp_init_t client;

		int train_id = atoi(argv[1]);

		int chemin_id = train_id;
		
        printf("EVC [%d] - Initialisation\n", train_id);

		setup_udp_client(&client, argv[2], atoi(argv[3]));

		can_socket = init_can_socket();
		if (can_socket == -1) {
			fprintf(stderr, "L'initialisation de la connexion au bus CAN a échouée\n");
			exit(EXIT_FAILURE);
		}

		install_signal_deroute(SIGINT, stop_train);
		
		report_position_args_t rpa = {client, train_id, &pos, &pos_mutex};

		pthread_create(&posreport_tid, NULL, report_position, &rpa);
		pthread_detach(posreport_tid);

		eoa_handler_args_t eha = {client, train_id, chemin_id, &eoa, &eoa_mutex,  &pos, &pos_mutex};

		pthread_create(&eoa_tid, NULL, eoa_handler, &eha);
		pthread_detach(eoa_tid);

		consigne_t consigne = {&destination, &dest_mutex, &max_speed, chemin_id};

		boucle_automatique(&pos, &pos_mutex, consigne, can_socket);

    exit(EXIT_SUCCESS);
    }
}

void stop_train() {
	printf("\nArrêt de l'EVC...\n");
	mc_consigneVitesse(can_socket, 0);
	fflush(stdout);
	close(can_socket);
	exit(EXIT_SUCCESS);
	return;
}

void install_signal_deroute(int numSig, void (*pfct)(int)) {
	struct sigaction newAct;
	newAct.sa_handler = pfct;
	CHECK_ERROR(sigemptyset (&newAct.sa_mask), -1, "-- sigemptyset() --");
	newAct.sa_flags = SA_RESTART;
	CHECK_ERROR(sigaction (numSig, &newAct, NULL), -1, "-- sigaction() --");
}