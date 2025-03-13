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

#include "../../utility/include/comm.h"
#include "../../utility/include/const_chemins.h"
#include "../../utility/include/can_infra.h"
#include "../../utility/include/can.h"
#include "../include/requests_handler.h"
#include "../include/trains.h"

typedef struct {
	int sd;
	struct sockaddr_in client_adr;
	message_t recv_message;
	int can_socket;
} handle_and_respond_args_t;

void * handle_and_respond(void * args);

int main(int argc, char *argv[]) {
    if (argc != 2) printf("RBC | Utilisation : ./rbc <port serveur> \n");
    else {
        
		pthread_t tid;

        printf("RBC - Initialisation\n");

		init_trains();

        int sd = setup_udp_server(atoi(argv[1]));

		int can_socket = init_can_socket();

        while (1) {
			struct sockaddr_in client_adr; 
			message_t recv_message;

            receive_data(sd, &client_adr, &recv_message);

			handle_and_respond_args_t hra = {sd, client_adr, recv_message, can_socket};

            pthread_create(&tid, NULL, handle_and_respond, &hra);
			pthread_detach(tid);
        }

    exit(EXIT_SUCCESS);
    }
}

/**
 * @brief Handles a request and sends a response. Entry point for a new thread.
 * @param args A handle_and_respond_args_t struct containing the received message, the client address and the socket descriptor
 */
void * handle_and_respond(void * args) {
	handle_and_respond_args_t * hra = (handle_and_respond_args_t *) args;
	message_t send_message;

	handle_request(hra->recv_message, &send_message, hra->can_socket);

	send_data(hra->sd, hra->client_adr, send_message);

	pthread_exit(NULL);

	return NULL;
}

