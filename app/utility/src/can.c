#include <arpa/inet.h>
#include <linux/can.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

// These files were initially included but dont seem useful :
// #include <fcntl.h>
// #include <linux/can/raw.h>
// #include <stdlib.h>
// #include <sys/socket.h>
// #include <time.h>

int send_can_data(int can_id, int can_dlc, int data[8]){
	   // Créer un socket CAN
    int sock;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;

    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        // perror("socket");
        return 1;
    }

    // Spécifier l'interface can0
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Associer le socket à l'interface can0
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Préparer le message CAN (ID 0x17 et données 0x0001)
    frame.can_id = can_id;  // Identifiant du message CAN
    frame.can_dlc = can_dlc;    // Taille des données (2 octets)
    for (int i = 0;i<8;i++){
        frame.data[i] = data[i];
    }

    // Envoyer le message
    if (write(sock, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("write");
        return 1;
    }	

    printf("Message CAN envoyé avec succès sur can0\n");

    close(sock);
    return 0;
}