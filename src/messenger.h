#ifndef MESSENGER_H
#define MESSENGER_H

#include "global.h"
#include "server.h"
#include "connection.h"

#define MESSENGER_SERVER_PORT 2020
#define THREAD_LOOP_TIME 100 // ms

#define MSGTYPE_USERNAME        0
#define MSGTYPE_USERNAME_ANSWER 1
#define MSGTYPE_MSG             2

typedef struct {
    pthread_t thread;
    SERVER server;

    // Username
    char username[32];

    // Connections
    int numConn;
    CONNECTION **conn;
    pthread_mutex_t mutex;
} MESSENGER;

typedef struct {
    MESSENGER *messenger;
    CONNECTION *conn;
} PTHREAD_CONN_ARG;

// Messenger manipulation and threads
void messenger_init(MESSENGER *messenger);
void messenger_destroy(MESSENGER *messenger);
void messenger_start(MESSENGER *messenger);
void messenger_stop(MESSENGER *messenger);

void messenger_run(MESSENGER *messenger);
void messenger_conn_run(PTHREAD_CONN_ARG *args);
void messenger_stopConn(MESSENGER *messenger, int pos);

// Menu
void messenger_menu(MESSENGER *messenger);
void messenger_menu_addContact(MESSENGER *messenger);
void messenger_menu_listContacts(MESSENGER *messenger);
void messenger_menu_deleteContact(MESSENGER *messenger);
void messenger_menu_sendMessage(MESSENGER *messenger);
void messenger_menu_sendGroupMessage(MESSENGER *messenger);
void messenger_menu_checkMessages(MESSENGER *messenger);
int messenger_menu_chooseContact(MESSENGER *messenger);

// Connections
int messenger_conn_connected2(MESSENGER *messenger, char ip[]);
CONNECTION* messenger_conn_getConnByIP(MESSENGER *messenger, char ip[]);
CONNECTION* messenger_conn_getConnByPos(MESSENGER *messenger, int pos);
int messenger_conn_getConnPosByIP(MESSENGER *messenger, char ip[]);
void messenger_conn_add(MESSENGER *messenger, CONNECTION *conn);
void messenger_conn_remove(MESSENGER *messenger, int pos);

// Messages
int messenger_msg_encode(char msgType, char *data, int size, char dest[]);

#endif // MESSENGER_H
