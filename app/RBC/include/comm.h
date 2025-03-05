#ifndef _UNIRAIL_COMM_H
	#define _UNIRAIL_COMM_H

	#define MAXOCTETS 150
	#define MAXDATA 2
	#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
										{ perror(msg); \
											exit(EXIT_FAILURE); }
	
	#include <arpa/inet.h>
	#include <netinet/in.h>

	typedef struct {
		int id;
		int code;
		char *data[MAXDATA];
	} message_t;

	typedef struct {
		int sd;
		struct sockaddr_in adr_serv;
		socklen_t adr_serv_len;
	} client_udp_init_t;

	int setup_udp_server(int port);
	void setup_udp_client(client_udp_init_t * client, char * server_ip, int server_port);
	void send_data(int sd, struct sockaddr_in send_adr, message_t message);
	void receive_data(int sd, struct sockaddr_in * recv_adr, message_t *message);
#endif