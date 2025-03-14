#include "../include/response_listener.h"
#include "../../utility/include/comm.h"

// Linked list of waiting responses
response_t *response_list = NULL;
pthread_mutex_t response_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void wait_for_response(int req_id, message_t *recv_message, int timeout_sec) {
    struct timespec ts;
	// Allocate a new response struct
    response_t *response = malloc(sizeof(response_t));
    if (!response) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

	// Initialize a waiting response struct
    response->req_id = req_id;
    response->ready = 0;
    pthread_cond_init(&response->cond, NULL);
    pthread_mutex_init(&response->mutex, NULL);
    response->next = NULL;


	// Add it at the beginning of the linked list
    pthread_mutex_lock(&response_list_mutex);
    response->next = response_list;
    response_list = response;
    pthread_mutex_unlock(&response_list_mutex);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_sec;
	// Awaits for the condition to be signaled
    pthread_mutex_lock(&response->mutex);
    while (!response->ready) {
        if (timeout_sec == 0) {
            pthread_cond_wait(&response->cond, &response->mutex);
        } else {
            int result = pthread_cond_timedwait(&response->cond, &response->mutex, &ts);
            if (result == ETIMEDOUT) {
                response->message.code = -1;
                response->message.data[0] = NULL;
                response->ready = 1;
                fprintf(stderr, "EVC - Pas de réponse reçue la requête %d après %d secondes\n", req_id, timeout_sec);
                break;
            }
        }
    }
    pthread_mutex_unlock(&response->mutex);
	
	// Copies the received message to the provided struct
	*recv_message = response->message;

	// Finds and removes the response from the linked list
    pthread_mutex_lock(&response_list_mutex);
    if (response_list == response) {
        response_list = response->next;
    } else {
        response_t *prev = response_list;
        while (prev && prev->next != response)
            prev = prev->next;
        if (prev)
            prev->next = response->next;
    }
    pthread_mutex_unlock(&response_list_mutex);

	// Cleanup
    pthread_cond_destroy(&response->cond);
    pthread_mutex_destroy(&response->mutex);
    free(response);

}
void dispatch_message(message_t *recv_message) {
        
    pthread_mutex_lock(&response_list_mutex);
    response_t *response = response_list;
	// Looks for the received message in the waiting list, adds it to the response struct and signals the condition
    while (response != NULL) {
        if (response->req_id == recv_message->req_id) {
            pthread_mutex_lock(&response->mutex);
            message_t * copied_message = copy_message(recv_message);
            response->message = *copied_message;
            free(copied_message);
            response->ready = 1;
            pthread_cond_signal(&response->cond);
            pthread_mutex_unlock(&response->mutex);
            break;
        }
        response = response->next;
    }
    pthread_mutex_unlock(&response_list_mutex);
}

void *response_listener(void * args) {
    message_t recv_message;
	response_listener_args_t *rla = (response_listener_args_t *) args;

    while (1) {
		
		receive_data(rla->client.sd, &rla->client.adr_serv, &recv_message);

        // Only case where the message is not a response but a new mission request
        if (recv_message.code == 103) {
            char * endptr; 
            long local_mission = strtol(recv_message.data[0], &endptr, 10); // Le pointeur n'a pas bougé, la conversion a échoué
            if (endptr == recv_message.data[0]) {
                fprintf(stderr, "EVC [%d] - Erreur lors de la réception d'une nouvelle mission: %s\n", recv_message.train_id, recv_message.data[0]);
                continue;
            }
            pthread_mutex_lock(rla->mission_mutex);
            *rla->mission = local_mission;
            pthread_mutex_unlock(rla->mission_mutex);

            message_t send_message;
            send_message.req_id = recv_message.req_id;
            send_message.train_id = recv_message.train_id;
            send_message.code = 203;
            send_message.data[0] = NULL;

            send_data(rla->client.sd, rla->client.adr_serv, send_message);

        // Otherwise, dispatch the message to the right thread waiting for it
        } else {
            dispatch_message(&recv_message);
        }

    }
    return NULL;
}
