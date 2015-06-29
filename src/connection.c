
#include "connection.h"

CONNECTION* connection_new(int socket, char ip[16], char name[32]) {
    CONNECTION *conn = malloc(sizeof(CONNECTION));
    conn->socket = socket;
    conn->numMessages = 0;
    conn->messages = NULL;
    conn->messagesTime = NULL;
    strcpy(conn->ip, ip);
    strcpy(conn->username, name);

    pthread_mutex_init(&(conn->mutex), NULL);
    return conn;
}

void connection_setUsername(CONNECTION *conn, char *username) {
    strcpy(conn->username, username);
}

void connection_pushMessage(CONNECTION *conn, char *msg) {
    pthread_mutex_lock(&(conn->mutex));

    const int newSize = conn->numMessages + 1;
    const int pos = conn->numMessages;

    // Realloc
    conn->messages = realloc(conn->messages, newSize*sizeof(char*));
    conn->messagesTime = realloc(conn->messagesTime, newSize*sizeof(time_t));

    // Alloc and copy
    conn->messages[pos] = calloc(strlen(msg)+1, sizeof(char));
    strcpy(conn->messages[pos], msg);

    time_t currTime;
    time(&currTime);
    conn->messagesTime[pos] = currTime;

    // Inc counter
    (conn->numMessages)++;

    pthread_mutex_unlock(&(conn->mutex));
}

void connection_popMessage(CONNECTION *conn, char msg[], time_t *time) {
    pthread_mutex_lock(&(conn->mutex));

    // Check if has messages
    if(!connection_hasMessages(conn))
        return;

    const int newSize = conn->numMessages - 1;

    // Copy and free
    strcpy(msg, conn->messages[0]);
    free(conn->messages[0]);

    *time = conn->messagesTime[0];

    // Shift vector and realloc
    int i=0;
    for(i=0; i<conn->numMessages-1; i++) {
        conn->messages[i] = conn->messages[i+1];
        conn->messagesTime[i] = conn->messagesTime[i+1];
    }
    conn->messages = realloc(conn->messages, newSize*sizeof(char*));
    conn->messagesTime = realloc(conn->messagesTime, newSize*sizeof(time_t));

    // Dec counter
    (conn->numMessages)--;

    pthread_mutex_unlock(&(conn->mutex));
}

int connection_hasMessages(CONNECTION *conn) {
    return (conn->numMessages!=0);
}
