#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
                                    { perror(msg); \
                                        exit(EXIT_FAILURE); }

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "../include/comm.h"
#include "../include/gestionnaire_requetes.h"

typedef struct {
	int sd;
	struct sockaddr_in client_adr;
	message_t recv_message;
} handle_and_respond_args_t;

void * handle_and_respond(void * args);

int main(int argc, char *argv[]) {
    if (argc != 2) printf("RBC | Utilisation : ./rbc <port serveur> \n");
    else {
        
		pthread_t tid;

        printf("RBC - Initialisation\n");

        int sd = setup_udp_server(atoi(argv[1]));

        while (1) {
			struct sockaddr_in client_adr; 
			message_t recv_message;

            receive_data(sd, &client_adr, &recv_message);

			handle_and_respond_args_t hra = {sd, client_adr, recv_message};

            pthread_create(&tid, NULL, handle_and_respond, &hra);
			pthread_detach(tid);
        }

    exit(EXIT_SUCCESS);
    }
}

void * handle_and_respond(void * args) {
	handle_and_respond_args_t * hra = (handle_and_respond_args_t *) args;
	message_t send_message;

	handle_request(hra->recv_message, &send_message);

	send_data(hra->sd, hra->client_adr, send_message);

	pthread_exit(NULL);

	return NULL;
}

