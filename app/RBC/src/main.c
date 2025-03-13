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
#include <errno.h>

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

			handle_and_respond_args_t *hra = malloc(sizeof(handle_and_respond_args_t));
			if (!hra) {
				fprintf(stderr, "Erreur d'allocation lors du traitement de la requête : %s\n", strerror(errno));
				continue;
			}

			message_t *copied_message = copy_message(&recv_message);

			hra->sd = sd;
			hra->client_adr = client_adr;
			hra->recv_message = *copied_message;
			hra->can_socket = can_socket;

            pthread_create(&tid, NULL, handle_and_respond, hra);
			pthread_detach(tid);

			free(copied_message);
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


	#define _GNUSOURCE
	pid_t tid = gettid();
	printf("%d, %s, %d", tid, inet_ntoa(hra->client_adr.sin_addr), send_message.train_id);

	handle_request(hra->recv_message, &send_message, hra->can_socket);

	printf("%d, %s, %d", tid, inet_ntoa(hra->client_adr.sin_addr), send_message.train_id);

	send_data(hra->sd, hra->client_adr, send_message);

	printf("%d, %s, %d", tid, inet_ntoa(hra->client_adr.sin_addr), send_message.train_id);

	pthread_exit(NULL);

	return NULL;
}

