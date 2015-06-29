
#include "server.h"

void server_init(SERVER *server) {
    server->socket = -1;

    server->newConn = 0;
    server->newConnSockets = NULL;

    pthread_mutex_init(&(server->mutex), NULL);
}

void server_destroy(SERVER *server) {
    if(server->newConn!=0) {
        free(server->newConnSockets);
        server->newConnSockets = NULL;
        server->newConn = 0;
    }

    pthread_mutex_destroy(&(server->mutex));
}

int server_start(SERVER *server, int port) {

    // Check if socket is already created
    if(server->socket!=-1)
        server_shutdown(server);

    // Create socket
    server->socket = socket(AF_INET, SOCK_STREAM, 0); // IPV4, TCP, IP Protocol
    if(server->socket == -1)
        return -1;

    // Bind socket
    struct sockaddr_in serverConf;
    serverConf.sin_family = AF_INET;
    serverConf.sin_addr.s_addr = INADDR_ANY;
    serverConf.sin_port = htons(port);
    if(bind(server->socket, (struct sockaddr*)&serverConf, sizeof(serverConf)) == -1)
        return -1;

    // Listen
    if(listen(server->socket, 3) == -1) // queue of 3 pending connections
        return -1;

    // Create thread
    pthread_create(&(server->thread), NULL, (void*)&server_run, (void*)server);

    return 1;
}

void server_stop(SERVER *server) {
    // Close thread
    pthread_cancel(server->thread);
    pthread_join(server->thread, NULL);

    // Shutdown socket
    server_shutdown(server);
}

void server_run(SERVER *server) {

    // Server loop
    while(1) {

        // Accept connection
        struct sockaddr_in client;
        int clientSocketLen=0;
        int clientSocket = accept(server->socket, (struct sockaddr*)&client, (socklen_t*)&clientSocketLen); // blocking call

        // Check failed
        if(clientSocket==-1)
            continue;

        // Add to list
        server_addNewConnection(server, clientSocket);
    }

}

void server_shutdown(SERVER *server) {
    // Shutdown socket
    if(server->socket!=-1) {
        shutdown(server->socket, SHUT_RDWR);
        close(server->socket);
        server->socket = -1;
    }
}

void server_addNewConnection(SERVER *server, int sock) {
    const int newListSize = server->newConn + 1;
    const int newPos = server->newConn;

    pthread_mutex_lock(&(server->mutex));

    // Realloc list
    server->newConnSockets = realloc(server->newConnSockets, newListSize*sizeof(int));

    // Add element
    server->newConnSockets[newPos] = sock;

    // Inc counter
    (server->newConn)++;

    pthread_mutex_unlock(&(server->mutex));
}

int server_getNewConnections(SERVER *server, int *socks, int max) {

    // Check if has new connections
    if(!server_hasNewConnections(server))
        return 0;

    pthread_mutex_lock(&(server->mutex));

    // Size
    int size = server->newConn;
    if(server->newConn > max)
        size = max;

    // Copy to 'socks'
    int i=0;
    for(i=0; i<size; i++)
        socks[i] = server->newConnSockets[i];

    // Shift list and realloc
    int newListSize = (server->newConn-size);
    for(i=0; i<newListSize; i++)
        server->newConnSockets[i] = server->newConnSockets[size+i];
    if(newListSize==0) {
        free(server->newConnSockets);
        server->newConnSockets = NULL;
    } else {
        server->newConnSockets = realloc(server->newConnSockets, newListSize*sizeof(int));
    }

    // Update counter
    server->newConn = newListSize;

    pthread_mutex_unlock(&(server->mutex));

    return size;
}

int server_hasNewConnections(SERVER *server) {
    pthread_mutex_lock(&(server->mutex));
    int retn = (server->newConn!=0);
    pthread_mutex_unlock(&(server->mutex));

    return retn;
}
