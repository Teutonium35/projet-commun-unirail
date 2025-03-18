#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "../include/gestionnaire_ressources.h"


unsigned char resources = 0b000000; // 6 bits pour représenter les ressources
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void lock_ressources(unsigned char res_mask) {
    pthread_mutex_lock(&lock);
    
    while ((resources & res_mask) != 0) { // Attente jusqu'à disponibilité
        pthread_mutex_unlock(&lock);
        usleep(50000); // Petit délai pour éviter de bloquer le thread
        pthread_mutex_lock(&lock);
    }
    resources |= res_mask; // Prendre les ressources
    pthread_mutex_unlock(&lock);
}

void unlock_ressources(unsigned char res_mask) {
    pthread_mutex_lock(&lock);
    resources &= ~res_mask; // Libérer les ressources
    pthread_mutex_unlock(&lock);
}