#include "../include/can.h"
#include "../include/can_infra.h"

const commande_aig command_aig_t1_r1[] = {{0x11,1},{0x10,0}};
const commande_aig command_aig_t1_r4[] = {{0x1A,1},{0x19,1},{0x21,0},{0x20,0},{0x1D,1},{0x1E,1}};
const commande_aig * command_aig_t1[2] = {command_aig_t1_r1,command_aig_t1_r4};

const commande_aig command_aig_t2_r2[] = {{0x13,0},{0x14,0}};
const commande_aig command_aig_t2_r3[] = {{0x15,0},{0x16,0}};
const commande_aig command_aig_t2_r4[] = {{0x19,0},{0x21,1},{0x1F,0},{0x1B,1},{0x18,0}};
const commande_aig * command_aig_t2[3] = {command_aig_t2_r2,command_aig_t2_r3,command_aig_t2_r4};

const commande_aig command_aig_t3_r5[] = {{0x1B,0},{0x1F,1}};
const commande_aig command_aig_t3_r3[] = {{0x15,1},{0x16,0}};
const commande_aig command_aig_t3_r4[] = {{0x19,0},{0x21,0},{0x20,0},{0x1D,1},{0x1E,1}};
const commande_aig command_aig_t2_r1_2[] = {{0x10,0},{0x11,0},{0x12,1},{0x13,1},{0x14,1}};
const commande_aig * command_aig_t3[4] = {command_aig_t3_r5,command_aig_t3_r3,command_aig_t3_r4,command_aig_t2_r1_2};

const commande_aig ** all_command_aig[3] = {command_aig_t1,command_aig_t2,command_aig_t3};



/// @brief Commande l'ensemble des aiguillages qui sont sensée être modifié pour le passage du train num_train lors du passage de next_bal_index
/// @param num_train numero du train, entre 0 et 2
/// @param next_bal_index index du chemin correspondant à la balise qu'on vient de rejoindre
/// @param can_socket can
/// @return 1 en cas d'erreur, 0 sinon
int set_all_switch(int num_train, int next_bal_index, int can_socket){
    const commande_aig * list_command = all_command_aig[num_train][next_bal_index];
    int longueur = sizeof(list_command)/sizeof(commande_aig);
    int result;
    for(int i = 0; i<longueur; i++){
        commande_aig command = list_command[i];
        if(command.dir == 1) result = set_switch_turn(command.name,can_socket);
        else result = set_switch_straight(command.name,can_socket);
        if(result) return 1;
    }
    return 0;
}

int set_switch_straight(int aig_name, int can_socket){
    int can_dlc = 3;
    int command_mode = 'W'; // For positioning, L to lock, U to unlock, I to reset
    int turn_mode = 0x01; // For straight, 0x00 for turn
    int data[8] = {command_mode,turn_mode,0,0,0,0,0,0};
    return send_can_data(can_socket, aig_name, can_dlc, data);
}

int set_switch_turn(int aig_name, int can_socket){
    int can_dlc = 3;
    int command_mode = 'W'; // For positioning, L to lock, U to unlock, I to reset
    int turn_mode = 0x00; // For turn, 0x01 for straight
    int data[8] = {command_mode,turn_mode,0,0,0,0,0,0};
    return send_can_data(can_socket, aig_name, can_dlc, data);
}
