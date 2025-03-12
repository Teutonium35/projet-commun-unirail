#ifndef _UNIRAIL_AUTOMATIQUE_H
	#define _UNIRAIL_AUTOMATIQUE_H
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <linux/can.h>
	#include <sys/socket.h>
	#include <net/if.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <linux/can.h>
	#include <linux/can/raw.h>
	#include <fcntl.h>
	#include <time.h>
	#include <pthread.h>

	#include "../../utility/include/map.h"

	#define PAS_ROUE_CODEUSE 0.016944 // en cm

	/**
	 * @struct consigne_t
	 * @brief Structure pour stocker la consigne de vitesse et de destination.
	 * @var consigne_t::destination Pointeur vers la position à rejoindre.
	 * @var consigne_t::destination_lock Mutex pour l'écriture/lecture de la position.
	 * @var consigne_t::max_speed Vitesse maximale autorisée.
	 * @var consigne_t::chemin_id Identifiant du chemin à suivre.
	 */
	typedef struct {
		position_t * destination;
		pthread_mutex_t * destination_lock;
		int * max_speed;
		int chemin_id;
	} consigne_t;

	/** 
	 *	@brief Donne la consigne en vitesse à partir de l'etat actuel du train.
	*	@param state état du train, composée de [distance à la destination,vitesse_actuelle] en [cm,cm/s].
	*	@return La commande en vitesse (en cm/s) nécessaire pour arriver à destination sans dépassement et en respectant les courbes de freinage (TODO).
	**/
	int compute_new_speed(int state[2]);

	/** 
	 *	@brief Envoie une commande sur le bus CAN de l'EVC pour commander la vitesse.
	*	@param can_socket socket du bus CAN de l'EVC.
	*	@param v vitesse demandée en cm/s.
	*	@return valeur d'erreur (1 si erreur, sinon 0).
	**/
	int mc_consigneVitesse(int can_socket, int v);

	/** 
	 *	@brief Récupère la valeur de la vitesse dans le frame du can_frame.
	 *	@param frame frame récuperée sur le can0 de l'EVC avec read(sock, &frame, sizeof(struct can_frame).
	 *	@return vitesse en cm/s.
	**/
	int read_speed_from_frame(struct can_frame frame);

	/** 
	 *	@brief Récupère la valeur de la position dans le frame du can_frame.
	 *	@param frame frame récuperée sur le can0 de l'EVC avec read(sock, &frame, sizeof(struct can_frame).
	 *	@return position relative à la dernière balise detectée (calculée par endométrie).
	**/
	float read_relative_pos_from_frame(struct can_frame frame);

	int get_min_speed(position_t pos_current, const int chemin_id);

	/** 
	 *	@brief Initie l'automatique du train en le faisant avancer jusqu'a la première balise (afin qu'il connaisse sa position initiale).
	 *	@param can_socket socket du bus CAN de l'EVC.
	 *	@return numéro de la première balise rencontrée.
	**/
	int init_train(int can_socket);

	/** 
	 *	@brief Fonction qui tourne en permanence sur l'EVC, afin de faire avancer le train jusqu'a la destion pointée par pos_destination. Actualise aussi la position actuelle en permanence.
	 *  @param position pointeur vers la position à rejoindre.
	 *  @param position_lock mutex pour l'écriture/lecture de position
	 *  @param consigne consigne de vitesse et de destination (et mutex).
	 *  @param can_socket socket du bus CAN de l'EVC.
	 *	@return valeur d'erreur (1 si erreur, sinon 0
	**/
	void * boucle_automatique(position_t * position, pthread_mutex_t * position_lock, consigne_t consigne, const int can_socket);
#endif