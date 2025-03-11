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
		int req_id;
		int train_id;
		int code;
		char *data[MAXDATA];
	} message_t;

	typedef struct {
		int sd;
		struct sockaddr_in adr_serv;
		socklen_t adr_serv_len;
	} client_udp_init_t;

	/**
	 *	@brief Generates a unique request ID
	 *	@return The generated request ID
	**/
	int generate_unique_req_id();

	/** 
	 *	@brief Creates and binds a UDP server socket
	 *	@param port The port to bind the server socket to
	 *	@return The file descriptor of the server socket 
	**/
	int setup_udp_server(int port);

	/** 
	 *	@brief Creates a UDP client socket and initializes the client struct
	 *	@param client The client_udp_init_t struct to initialize
	 *	@param server_ip The IP address of the server
	 *	@param server_port The port of the server
	**/
	void setup_udp_client(client_udp_init_t * client, char * server_ip, int server_port);

	/** 
	 *	@brief Sends a message to the server
	 *	@param sd The file descriptor of the client socket
	 *	@param send_adr The address of the server
	 *	@param message The message struct to send
	**/
	void send_data(int sd, struct sockaddr_in send_adr, message_t message);

	/**
	 * 	@brief Awaits a message (blocking)
	 * 	@param sd The file descriptor of the socket to use
	 *  @param recv_adr The address in which to store the sender's address
	 *  @param message The message struct to store the received message
	 */
	void receive_data(int sd, struct sockaddr_in * recv_adr, message_t *message);
#endif