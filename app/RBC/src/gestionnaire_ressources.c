#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define NB_RESSOURCES 6
#define PORT 8080

unsigned char resources = 0b000000; // 6 bits pour représenter les ressources
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    unsigned char request;

    while (recv(client_socket, &request, 1, 0) > 0) {
        int action = (request & 0b10000000) >> 7; // Premier bit (1 = prise, 0 = libération)
        unsigned char res_mask = request & 0b01111111; // 6 bits restants

        pthread_mutex_lock(&lock);
        
        if (action == 1) { // Prise de ressources
            while ((resources & res_mask) != 0) { // Attente jusqu'à disponibilité
                pthread_mutex_unlock(&lock);
                usleep(50000); // Petit délai pour éviter de bloquer le thread
                pthread_mutex_lock(&lock);
            }
            resources |= res_mask; // Prendre les ressources
        } else { // Libération de ressources
            resources &= ~res_mask; // Libérer les ressources
        }
        
        pthread_mutex_unlock(&lock);
        send(client_socket, "OK", 2, 0);
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, *client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("Serveur en attente de connexions...");
    
    while (1) {
        client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_socket);
        pthread_detach(thread);
    }
    
    close(server_socket);
    return 0;
}
