#include "../../utility/include/can.h"
#include "../../utility/include/can_infra.h"
#include <unistd.h>

int main(){
    int RC = 0x1F2;
    
    set_switch_straight(RC);

    sleep(3);

    set_switch_turn(RC);

    sleep(3);

    set_switch_straight(RC);

}
