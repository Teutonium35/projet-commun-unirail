#include "init.h"
#include <signal.h>

//#define VERSION_SOFT 0x62
// #define RESEARCH

static volatile int running = 1, canID, socketID1, socketID2;

void sigterm(int signo)
{
    printf("\n\n..........Stopping application..........\n\n");
    int i;
    for (i = 20; i>-1; i--)
    {
     usleep(50000);
    }
	signal(SIGINT, SIG_DFL);
	raise(SIGINT);
}

int sockfd, newsockfd;
int portno = 2055;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

float vitesseMaximaleAutorisee ;
float vitesseUrgence ;

int Reception_balise = 0;

int Mem_Etat_Feux = 0;

double * parametres;

uCAN1_MSG recCanMsg;


/**
 Fonctions
*/

////////////////////////////////////////
/// Thread scheduleur
///////////////////////////////////////
void *scheduleur(void *arg)
{
/*
	int canPort, i_debug =0, meancounter = 0;
	char *NomPort = "can0";
	struct timespec startTime, stopTime;
	uCAN1_MSG trameScheduleur, trameStatusRun;
	trameScheduleur.frame.id = MC_ID_SCHEDULEUR_MESURES;
	trameScheduleur.frame.dlc = MC_DLC_SCHEDULEUR_MESURES;

	canPort = canLinux_init_prio(NomPort);
	canID = canPort;
	while (1)
	{
		pthread_mutex_lock(&train_lock);
		clock_gettime(CLOCK_REALTIME,&startTime);
		accelererConsigne(&locoInfo);
		fillTrameScheduleur(&trameScheduleur, &locoInfo);
		canLinux_transmit(CANLINUX_PRIORITY_HIGH, &trameScheduleur);
		clock_gettime(CLOCK_REALTIME,&stopTime);
		
		canLinux_transmit(CANLINUX_PRIORITY_HIGH, &trameStatusRun);
	
		pthread_mutex_unlock(&train_lock);
	}
	close(canPort);
   */
  usleep(10);
	return NULL;
}

////////////////////////////////////////
/// Thread monitor
///////////////////////////////////////
void *monitor(void *arg)
{

  usleep(10);
	return NULL;
}

////////////////////////////////////
/// Thread Ecoute BUS CAN
////////////////////////////////////
void *ecoute (void *arg) 
{
	// declaration des variables
	int canPort;
	char *NomPort = "vcan0";
	
	canPort = canLinux_init_prio(NomPort);
	canID = canPort;
	uint8_t i = 0;
 
	while(1)
	{
		/** Affichage de trame reÃƒÂ§ue **/
		if(canLinux_receive(&recCanMsg, 1))
		{
			pthread_mutex_lock(&train_lock);
			
			printf("Nouvelle ID CAN en hexa : %X avec un DLC : %X \n", recCanMsg.frame.id,recCanMsg.frame.dlc);
			  
			pthread_mutex_unlock(&train_lock);
		}
    usleep(1000);
	}
	close(canPort);
	return NULL;
}

////////////////////////////////////////
/// MAIN
////////////////////////////////////////
int main(void)
{

	//SIGHUP 	Rupture detectee sur le terminal controleur ou mort du processus parent 
	//SIGINT 	Interruption du clavier (Ctrl+C dans un terminal) 
	//SIGHUP 	Ecriture TCP sur un socket ferme (deconnexion) 

	signal(SIGINT, sigterm);
	signal(SIGPIPE, sigterm);
	signal(SIGHUP, sigterm);

	
    printf("\n\n..........Starting application..........\n\n");

	usleep(100000);
	pthread_t threadEcoute, threadMonitor, threadScheduleur;
	pthread_mutex_init(&train_lock, NULL);

	/* thread create */
	if(pthread_create(&threadEcoute, NULL, ecoute, NULL) == -1)
	{
		perror("pthread_create ecoute");
		return EXIT_FAILURE;
	}
	
	if(pthread_create(&threadMonitor, NULL, monitor, NULL) == -1)
	{
		perror("pthread_create monitor");
		return EXIT_FAILURE;
	}
	
	if(pthread_create(&threadScheduleur, NULL, scheduleur, NULL) == -1)
	{
		perror("pthread_create Scheduleur");
		return EXIT_FAILURE;
	}

	/* thread join */
	if (pthread_join(threadEcoute, NULL))
	{
		perror("pthread_join ecoute");
		return EXIT_FAILURE;
	}

	if (pthread_join(threadMonitor, NULL))
	{
		perror("pthread_join monitor");
		return EXIT_FAILURE;
	}

	if (pthread_join(threadScheduleur, NULL))
	{
		perror("pthread_join Scheduleur");
		return EXIT_FAILURE;
	}
	return 0;
}
