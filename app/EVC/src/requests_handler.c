#include "../include/requests_handler.h"

void * requests_handler(void * args) {
    requests_handler_args_t * rha = (requests_handler_args_t *) args;
    message_t send_message;
    while (1) {

        pthread_mutex_lock(&rha->pending_request.mutex);
        while (!rha->pending_request.ready) {
            pthread_cond_wait(&rha->pending_request.cond, &rha->pending_request.mutex);
        }

        handle_request(rha->pending_request.message, &send_message, rha->mission_mutex, rha->mission);

        send_data(rha->sd, *rha->pending_request.recv_adr, send_message);

        free(rha->pending_request.recv_adr);

        rha->pending_request.ready = 0;
        rha->pending_request.message = (message_t) {0, 0, 0, {NULL, NULL}};
        
        pthread_mutex_unlock(&rha->pending_request.mutex);
    }
}

void handle_request(message_t recv_message, message_t * send_message, pthread_mutex_t * mission_mutex, int * mission) {
    
    send_message->train_id = recv_message.train_id;
    send_message->req_id = recv_message.req_id;

    switch (recv_message.code) {
        case 103: {
            char * endptr;
            long local_mission = strtol(recv_message.data[0], &endptr, 10); // Le pointeur n'a pas bougé, la conversion a échoué
            if (endptr == recv_message.data[0]) {
                fprintf(stderr, "EVC [%d] - Erreur lors de la réception d'une nouvelle mission: %s\n", recv_message.train_id, recv_message.data[0]);
                send_message->code = 403;
                send_message->data[0] = NULL;   
            }
            pthread_mutex_lock(mission_mutex);
            *mission = local_mission;
            pthread_mutex_unlock(mission_mutex);
            break;
        }
        default: {
            fprintf(stderr, "EVC [%d] - Code de requête inconnu: %d\n", recv_message.train_id, recv_message.code);
            send_message->code = 400;
            send_message->data[0] = NULL;
            break;
        }
    }
}
