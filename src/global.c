#include "global.h"

void socket2ip(int socket, char retn[17]) {
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    getpeername(socket, (struct sockaddr*)&client, &len);

    inet_ntop(AF_INET, &(client.sin_addr), retn, 17);
}
