#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>      
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../include/comm.h"
#include "../include/debug.h"

void setup_udp_client(client_udp_init_t * client, char * server_ip, int server_port) {

	client->sd=socket(AF_INET,SOCK_DGRAM,0); 
	CHECK_ERROR(client->sd,-1, "EVC - Erreur lors de la création du socket de dialogue\n");
	printf("EVC - N° du socket de dialogue : %d \n", client->sd);
	
	// Préparation de l'adresse
	client->adr_serv.sin_family=AF_INET;
	client->adr_serv.sin_port=htons(server_port);
	client->adr_serv.sin_addr.s_addr=inet_addr(server_ip);

	// Affectation de l'adresse au socket
	client->adr_serv_len = sizeof(client->adr_serv);
	
	return;
}

int setup_udp_server(int port) {
	int sd, erreur;
	struct sockaddr_in serv_adr;
	socklen_t serv_adr_len = sizeof(serv_adr);
	sd=socket(AF_INET,SOCK_DGRAM,0); 
	CHECK_ERROR(sd,-1, "RBC - Erreur lors de la création du socket  de dialogue\n");
	printf("RBC - N° du socket de dialogue : %d \n", sd);
	
	// Préparation de l'adresse
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_port=htons(port);
	serv_adr.sin_addr.s_addr=inet_addr("0.0.0.0");

	// Affectation de l'adresse au socket
	erreur=bind(sd,(const struct sockaddr *) &serv_adr, serv_adr_len);
	CHECK_ERROR(erreur,-1, "RBC - Erreur lors du bind du socket de dialogue\n");
	
	return sd;
}

void send_data(int sd, struct sockaddr_in send_adr, message_t message) {
    char buff_send[MAXOCTETS + 1];
    char data_str[MAXOCTETS - 50] = ""; // Pour construire le str data à partir de la liste de datas
	int nbcar;
	socklen_t send_adr_len = sizeof(send_adr);

    for (int i = 0; message.data[i] != NULL && i < MAXDATA; i++) {
        if (strlen(data_str) + strlen(message.data[i]) + 1 < sizeof(data_str)) {
			strcat(data_str, message.data[i]);
            strcat(data_str, ":");
        } else break;
    }

    sprintf(buff_send, "%d:%d:%s", message.id, message.code, data_str);
	DEBUG_PRINT("Envoi de données : %s\n", buff_send);
    
    nbcar=sendto(sd, buff_send, strlen(buff_send) + 1, 0, (struct sockaddr *)&send_adr, send_adr_len);
	CHECK_ERROR(nbcar,0,"\nErreur lors de l'émission des données");
}

void receive_data(int sd, struct sockaddr_in * recv_adr, message_t *message) {
	int nbcar;
	char * ptr;
	socklen_t recv_adr_len = sizeof(*recv_adr);
	char buff_recv[MAXOCTETS + 1];

	DEBUG_PRINT("Attente de données...\n");
	
	nbcar = recvfrom(sd, buff_recv, MAXOCTETS + 1, 0, (struct sockaddr *)recv_adr, &recv_adr_len);
	CHECK_ERROR(nbcar,0,"\nErreur lors de la réception des données");
	buff_recv[nbcar] = '\0';

	DEBUG_PRINT("Réception de données : %s\n", buff_recv);

	ptr = strtok(buff_recv, ":");
	if (ptr == NULL) {
		message->id = -1;
		message->code = 500;
		fprintf(stderr, "Erreur: message malformée\n");
		return;
	}
	message->id = atoi(ptr);

	ptr = strtok(NULL, ":");
	if (ptr == NULL) {
		message->code = 500;
		fprintf(stderr, "Erreur: message malformée\n");
		return;
	}
	message->code = atoi(ptr);

	ptr = strtok(NULL, ":");
	for (int i = 0; ptr != NULL && i < MAXDATA; i++) {
		message->data[i] = malloc(strlen(ptr) + 1);
		if (message->data[i] == NULL) {
			perror("Memory allocation error");
			exit(EXIT_FAILURE);
		}
		strcpy(message->data[i], ptr);
		ptr = strtok(NULL, ":");
	}
	return;
}