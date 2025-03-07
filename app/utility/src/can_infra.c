#include "../include/can.h"

int set_switch_straight(int aig_name){
    int can_dlc = 3;
    int command_mode = 'W'; // For positioning, L to lock, U to unlock, I to reset
    int turn_mode = 0x01; // For straight, 0x00 for straight
    int data[8] = {command_mode,turn_mode,0,0,0,0,0,0};
    return send_can_data(aig_name, can_dlc, data);
}

int set_switch_turn(int aig_name){
    int can_dlc = 3;
    int command_mode = 'W'; // For positioning, L to lock, U to unlock, I to reset
    int turn_mode = 0x00; // For turn, 0x01 for straight
    int data[8] = {command_mode,turn_mode,0,0,0,0,0,0};
    return send_can_data(aig_name, can_dlc, data);
}
