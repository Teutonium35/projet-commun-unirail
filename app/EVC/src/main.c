#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
                                    { perror(msg); \
                                        exit(EXIT_FAILURE); }

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

#include "../include/position.h"
#include "../include/automatique.h"
#include "../include/eoa_handler.h"
#include "../include/response_listener.h"
#include "../../utility/include/can.h"

position_t pos = {0, 0.0};
position_t eoa = {-1, 0.0};
position_t destination = {3, 20.0};


pthread_mutex_t pos_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dest_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t eoa_mutex = PTHREAD_MUTEX_INITIALIZER;

int pos_initialized = 0;
pthread_mutex_t init_pos_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init_pos_cond = PTHREAD_COND_INITIALIZER;

int eoa_initialized = 0;
pthread_mutex_t init_eoa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init_eoa_cond = PTHREAD_COND_INITIALIZER;

int can_socket;
int mission = 0;
pthread_mutex_t mission_mutex = PTHREAD_MUTEX_INITIALIZER;

void install_signal_deroute(int numSig, void (*pfct)(int));
void stop_train();

int main(int argc, char *argv[]) {
    if (argc != 5) printf("EVC | Utilisation : ./rbc <id train> <nombre de tours> <adresse_serveur> <port serveur> \n");
    else {
        
		pthread_t posreport_tid, eoa_tid, response_listener_tid;
		client_udp_init_t client;
		message_t init_message;

		int train_id = atoi(argv[1]);

		int chemin_id = train_id;

		mission = atoi(argv[2]);
		
        printf("EVC [%d] - Initialisation\n", train_id);

		setup_udp_client(&client, argv[3], atoi(argv[4]));

		can_socket = init_can_socket();
		if (can_socket == -1) {
			fprintf(stderr, "L'initialisation de la connexion au bus CAN a échouée\n");
			exit(EXIT_FAILURE);
		}

		install_signal_deroute(SIGINT, stop_train);

		init_message.req_id = generate_unique_req_id();
		init_message.train_id = train_id;
		init_message.code = 100;
		init_message.data[0] = NULL;

		printf("EVC [%d] - En attente du RBC...", train_id);

		send_data(client.sd, client.adr_serv, init_message);

		receive_data(client.sd, NULL, &init_message);

		if (init_message.code != 200) {
			fprintf(stderr, "EVC [%d] - Erreur lors de la connexion: %d\n", train_id, init_message.code);
			exit(EXIT_FAILURE);
		}

		printf("OK\nEVC [%d] - Connexion établie\n", train_id);

		response_listener_args_t rla = {client, &mission, &mission_mutex};

		pthread_create(&response_listener_tid, NULL, response_listener, &rla);
		pthread_detach(response_listener_tid);
		
		report_position_args_t rpa = {client, train_id, &pos, &pos_mutex, &pos_initialized, &init_pos_cond, &init_pos_mutex};

		pthread_create(&posreport_tid, NULL, report_position, &rpa);
		pthread_detach(posreport_tid);

		eoa_handler_args_t eha = {client, train_id, chemin_id, &eoa, &eoa_mutex,  &pos, &pos_mutex, 
			&pos_initialized, &init_pos_cond, &init_pos_mutex,
			&eoa_initialized, &init_eoa_cond, &init_eoa_mutex};

		pthread_create(&eoa_tid, NULL, eoa_handler, &eha);
		pthread_detach(eoa_tid);

		boucle_automatique_args_t baa = {&pos, &pos_mutex, &eoa, &eoa_mutex, &mission, &mission_mutex,
			chemin_id, can_socket, &eoa_initialized, &init_eoa_cond, &init_eoa_mutex};

		boucle_automatique(&baa);

    exit(EXIT_SUCCESS);
    }
}

void stop_train() {
	printf("\nArrêt de l'EVC...\n");
	mc_consigneVitesse(can_socket, 0);
	fflush(stdout);
	close(can_socket);
	exit(EXIT_SUCCESS);
}

void install_signal_deroute(int numSig, void (*pfct)(int)) {
	struct sigaction newAct;
	newAct.sa_handler = pfct;
	CHECK_ERROR(sigemptyset (&newAct.sa_mask), -1, "-- sigemptyset() --");
	newAct.sa_flags = SA_RESTART;
	CHECK_ERROR(sigaction (numSig, &newAct, NULL), -1, "-- sigaction() --");
}