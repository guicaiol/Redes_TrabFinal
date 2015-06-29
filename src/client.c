
#include "client.h"

int client_connect(char *ip, int port) {

    // Create socket for connection
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // IPV4, TCP, IP Protocol
    if(clientSocket == -1)
        return -1;

    // Connect to target IP address
    struct sockaddr_in serverConf;
    serverConf.sin_addr.s_addr = inet_addr(ip);
    serverConf.sin_family = AF_INET;
    serverConf.sin_port = htons(port);

    // Connect
    if(connect(clientSocket, (struct sockaddr*)&serverConf, sizeof(serverConf)) == -1) {
        close(clientSocket);
        return -1;
    }

    // Connection estabilished
    return clientSocket;
}


int client_disconnect(int sock) {
    if(shutdown(sock, SHUT_RDWR)==-1)
        return -1;
    return close(sock);
}
