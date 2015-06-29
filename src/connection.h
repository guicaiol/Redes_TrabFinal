#ifndef CONNECTION_H
#define CONNECTION_H

#include "global.h"

typedef struct {
    pthread_t thread;

    int socket; // socket
    char ip[16]; // contact's IP address
    char username[32]; // contact's username

    int numMessages; // num of messages pending
    char **messages; // pending messages
    time_t *messagesTime; // recv time

    pthread_mutex_t mutex; // mutex
} CONNECTION;

CONNECTION* connection_new(int socket, char ip[16], char name[32]);

void connection_setUsername(CONNECTION *conn, char *username);

void connection_pushMessage(CONNECTION *conn, char *msg);
void connection_popMessage(CONNECTION *conn, char msg[], time_t *time);
int connection_hasMessages(CONNECTION *conn);

#endif // CONNECTION_H
