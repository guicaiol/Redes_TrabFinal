#ifndef SERVER_H
#define SERVER_H

#include "global.h"

typedef struct {
    pthread_t thread;
    int socket;

    pthread_mutex_t mutex;
    int newConn;
    int *newConnSockets;
} SERVER;

// Server manipulation
void server_init(SERVER *server);
void server_destroy(SERVER *server);
int server_start(SERVER *server, int port);
void server_stop(SERVER *server);

// Connections
int server_getNewConnections(SERVER *server, int *socks, int max);
int server_hasNewConnections(SERVER *server);
void server_addNewConnection(SERVER *server, int sock);

// Internal
void server_run(SERVER *server);
void server_shutdown(SERVER *server);

#endif // SERVER_H
