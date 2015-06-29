
#include "messenger.h"
#include "timer.h"
#include "client.h"

void messenger_init(MESSENGER *messenger) {
    messenger->numConn = 0;
    messenger->conn = NULL;

    // Server init
    server_init(&(messenger->server));

    // Mutex for thread-safe
    pthread_mutex_init(&(messenger->mutex), NULL);
}

void messenger_start(MESSENGER *messenger) {
    // Get user name
    printf(">> Welcome to Messenger!\n");
    printf(">> How should I call you? ");
    fgets(messenger->username, 32, stdin);
    messenger->username[strlen(messenger->username)-1] = '\0'; // remove \n

    // Start server for receiving connections
    if(server_start(&(messenger->server), MESSENGER_SERVER_PORT)==-1) {
        printf(">> Failed to start Messenger.\n>> Error: %s.\n", strerror(errno));
        return;
    }

    // Start connection handler thread
    pthread_create(&(messenger->thread), NULL, (void*)&messenger_run, (void*)messenger);

    // Start menu
    messenger_menu(messenger);
}

void messenger_run(MESSENGER *messenger) {
    TIMER t;

    // Messenger handler loop
    while(1) {
        timer_start(&t);

        // Check new incoming connections
        int socks[8];
        int n = server_getNewConnections(&(messenger->server), socks, 8);

        // Handle new connections
        pthread_mutex_lock(&(messenger->mutex));
        int i=0;
        for(i=0; i<n; i++) {
            const int sock = socks[i];

            // Get IP address
            char ip[16];
            socket2ip(sock, ip);

            // Create connection and add to list
            CONNECTION *newConn = connection_new(socks[i], ip, "Unknown contact");
            messenger_conn_add(messenger, newConn);

            // Start thread
            CONNECTION *conn = messenger_conn_getConnByIP(messenger, ip);

            PTHREAD_CONN_ARG args;
            args.messenger = messenger;
            args.conn = conn;
            pthread_create(&(conn->thread), NULL, (void*)messenger_conn_run, (void*)&args);
        }
        pthread_mutex_unlock(&(messenger->mutex));

        timer_stop(&t);

        // Loop time control
        double rest = THREAD_LOOP_TIME - timer_timemsec(&t);
        if(rest>0)
            msleep(rest);
    }
}

void messenger_conn_run(PTHREAD_CONN_ARG *args) {
    // Parse args
    MESSENGER *messenger = args->messenger;
    CONNECTION *conn = args->conn;

    // Buffer
    char recvBuffer[1024];
    int retn=0;

    // Connection handler thread
    while(1) {
        memset(recvBuffer, 0, 1024); // clear buffer

        // Recv messages
        retn = recv(conn->socket, recvBuffer, 1024, 0);

        pthread_mutex_lock(&(messenger->mutex));

        // Check errors
        if(retn==-1) {
            printf(">> MESSENGER: Failed to receive message (%s)!\n", strerror(errno));

            pthread_mutex_unlock(&(messenger->mutex));
            continue;
        } else if(retn==0) { // Disconnected: remove from contact list and end thread
            client_disconnect(conn->socket);
            int pos = messenger_conn_getConnPosByIP(messenger, conn->ip);
            messenger_conn_remove(messenger, pos);

            pthread_mutex_unlock(&(messenger->mutex));
            break;
        }

        // Handle message
        char msgType = recvBuffer[0];
        char *data = recvBuffer+1;
        switch(msgType) {
            case MSGTYPE_USERNAME: {
                // Update contact username
                char username[32];
                sscanf(data, "%s", username);
                connection_setUsername(conn, username);

                // Send back my username
                char sendBuffer[33];
                int msgSize = messenger_msg_encode(MSGTYPE_USERNAME_ANSWER, messenger->username, strlen(messenger->username), sendBuffer);
                send(conn->socket, sendBuffer, msgSize, 0);
            } break;

            case MSGTYPE_USERNAME_ANSWER: {
                // Update contact username
                char username[32];
                sscanf(data, "%s", username);
                connection_setUsername(conn, username);
            } break;

            case MSGTYPE_MSG: {
                connection_pushMessage(conn, data);
            } break;
        }

        pthread_mutex_unlock(&(messenger->mutex));
    }

}

void messenger_stop(MESSENGER *messenger) {
    // Stop messenger thread
    pthread_cancel(messenger->thread);
    pthread_join(messenger->thread, NULL);

    // Stop server
    server_stop(&(messenger->server));

    // Stop connections
    int i=0;
    for(i=0; i<messenger->numConn; i++)
        messenger_stopConn(messenger, i);
}

void messenger_stopConn(MESSENGER *messenger, int pos) {
    // Get conn
    CONNECTION *conn = messenger_conn_getConnByPos(messenger, pos);

    // Thread
    pthread_cancel(conn->thread);
    pthread_join(conn->thread, NULL);

    // Socket
    client_disconnect(conn->socket);

    // Remove from list
    messenger_conn_remove(messenger, pos);
}

void messenger_destroy(MESSENGER *messenger) {
    // Destroy server
    server_destroy(&(messenger->server));

    // Free connections
    int i=0;
    for(i=0; i<messenger->numConn; i++) {
        CONNECTION *conn = messenger_conn_getConnByPos(messenger, i);
        pthread_mutex_destroy(&(conn->mutex));
        free(conn);
    }

    pthread_mutex_destroy(&(messenger->mutex));
}

void messenger_menu(MESSENGER *messenger) {
    int running = 1;

    // Messenger menu loop
    while(running) {
        __fpurge(stdin);
        system("clear");
        printf("################# Main menu #################\n");
        printf("Hello, %s.\n\n", messenger->username);
        printf("1- Add contact\n2- Contact list\n3- Delete contact\n4- Send message\n5- Send group message\n6- Check new messages\n7- Quit\n");
        printf("\nChoose option: ");

        int option = getchar();
        __fpurge(stdin);
        if(option!='7')
            system("clear");

        pthread_mutex_lock(&(messenger->mutex));

        int invalidOption = 0;
        switch (option) {
            case '1':
                messenger_menu_addContact(messenger);
                break;
            case '2':
                messenger_menu_listContacts(messenger);
                break;
            case '3':
                messenger_menu_deleteContact(messenger);
                break;
            case '4':
                messenger_menu_sendMessage(messenger);
                break;
            case '5':
                messenger_menu_sendGroupMessage(messenger);
                break;
            case '6':
                messenger_menu_checkMessages(messenger);
                break;
            case '7':
                messenger_stop(messenger);
                running = 0;
                break;
            default:
                invalidOption = 1;
                break;
        }

        pthread_mutex_unlock(&(messenger->mutex));

        // Show only in valid options
        if(!invalidOption && option!='7') {
            printf("\nPress <ENTER> to go back to menu...");
            getchar();
        }

    }

    printf("\nSee you, %s!\n\n", messenger->username);
}

int messenger_menu_chooseContact(MESSENGER *messenger) {
    // Check no contacts
    if(messenger->numConn==0) {
        printf("You have no contacts!\n");
        printf("Use option (1) to add a contact.\n");
        return -1;
    }

    // Show contacts
    printf("Contact list:\n");
    int i;
    for(i=0; i<messenger->numConn; i++)
        printf("%d- %s (%s)\n", i+1, messenger->conn[i]->username, messenger->conn[i]->ip);

    // Choose contact
    int contact=0;
    do {
        __fpurge(stdin);
        printf("\n>> Choose contact (0 to exit): ");
        scanf("%d", &contact);
        __fpurge(stdin);
        if(contact==0)
            break;
    } while(contact>messenger->numConn);

    return contact-1;
}

void messenger_menu_addContact(MESSENGER *messenger) {
    printf("################# Add contact #################\n");
    printf("Type your contact's IP address (0 to exit): ");

    // Read contact IP address
    char ip[17];
    __fpurge(stdin);
    fgets(ip, 17, stdin);
    ip[strlen(ip)-1] = '\0';

    // Check exit
    if(strcmp(ip, "")==0 || strcmp(ip, "0")==0)
        return;

    printf(">> Connecting...\n");

    // Check if is already connected
    if(messenger_conn_connected2(messenger, ip)) {
        CONNECTION *conn = messenger_conn_getConnByIP(messenger, ip);
        printf(">> You are already connected to %s (%s).\n", conn->username, conn->ip);
        return;
    }

    CONNECTION *newConn=NULL;
    int connected = 0;

    // Connect
    int sock = client_connect(ip, MESSENGER_SERVER_PORT);
    if(sock>=0) {
        newConn = connection_new(sock, ip, "Unknown contact");
        connected = 1;
    }

    // If connected
    if(connected) {
        // Add to list
        messenger_conn_add(messenger, newConn);

        // Start thread
        CONNECTION *conn = messenger_conn_getConnByIP(messenger, ip);

        PTHREAD_CONN_ARG args;
        args.messenger = messenger;
        args.conn = conn;
        pthread_create(&(conn->thread), NULL, (void*)messenger_conn_run, (void*)&args);

        // Send username
        char sendBuffer[33];
        int msgSize = messenger_msg_encode(MSGTYPE_USERNAME, messenger->username, strlen(messenger->username), sendBuffer);
        send(conn->socket, sendBuffer, msgSize, 0);

        printf(">> Successfully connected.\n");

        return;
    }

    // Failed to connect
    printf(">> Failed to connect to %s!\n", ip);
    printf(">> The contact is probably offline.\n");
    printf(">> Error: %s.\n", strerror(errno));
}

void messenger_menu_listContacts(MESSENGER *messenger) {
    printf("################# Contact list #################\n");

    // Check no contatcs
    if(messenger->numConn==0) {
        printf("You have no contacts!\n");
        printf("Use option (1) to add a contact.\n");
        return;
    } else {
        printf("Here are your contacts, %s:\n\n", messenger->username);
    }

    // Show contacts
    int i;
    for(i=0; i<messenger->numConn; i++)
        printf("%d- %s (%s)\n", i+1, messenger->conn[i]->username, messenger->conn[i]->ip);
}

void messenger_menu_deleteContact(MESSENGER *messenger) {
    printf("################# Delete contact #################\n");

    // Choose contact
    int pos = messenger_menu_chooseContact(messenger);
    if(pos==-1)
        return;

    // Stop conn
    messenger_stopConn(messenger, pos);

    printf(">> Contact deleted.\n");
}

void messenger_menu_sendMessage(MESSENGER *messenger) {
    printf("################# Send message #################\n");

    // Choose contact
    int pos = messenger_menu_chooseContact(messenger);
    if(pos==-1)
        return;

    // Get connection
    CONNECTION *conn = messenger_conn_getConnByPos(messenger, pos);

    printf(">> Type message to %s (%s):\n", conn->username, conn->ip);
    printf(">> Press single <ENTER> to stop.\n");
    while(1) {

        // Read message to send
        __fpurge(stdin);
        char msg[128];
        fgets(msg, 128, stdin);
        msg[strlen(msg)-1] = '\0';
        __fpurge(stdin);

        // Check <ENTER>
        if(msg[0]=='\0')
            break;

        char sendBuffer[129];

        // Send
        int msgSize = messenger_msg_encode(MSGTYPE_MSG, msg, strlen(msg), sendBuffer);
        send(conn->socket, sendBuffer, msgSize, 0);
    }

    // Check retn
    printf(">> Messages sent.\n");
}

void messenger_menu_sendGroupMessage(MESSENGER *messenger) {
    printf("################# Send group message #################\n");

    // Check no contatcs
    if(messenger->numConn==0) {
        printf("You have no contacts!\n");
        printf("Use option (1) to add a contact.\n");
        return;
    }

    // Show contacts
    printf("Contact list:\n");
    int i;
    for(i=0; i<messenger->numConn; i++)
        printf("%d- %s (%s)\n", i+1, messenger->conn[i]->username, messenger->conn[i]->ip);

    // Choose contacts
    printf("\n>> Choose up to 5 contacts, separated by space (0 to exit): ");
    char buf[32];
    __fpurge(stdin);
    fgets(buf, 32, stdin);
    __fpurge(stdin);

    // Parse
    int contacts[5];
    int numGroup = sscanf(buf, "%d%d%d%d%d", &(contacts[0]), &(contacts[1]), &(contacts[2]), &(contacts[3]), &(contacts[4]));

    printf(">> Type message to group:\n");
    printf(">> Press single <ENTER> to stop.\n");
    while(1) {

        // Read message to send
        __fpurge(stdin);
        char msg[128];
        fgets(msg, 128, stdin);
        msg[strlen(msg)-1] = '\0';
        __fpurge(stdin);

        // Check <ENTER>
        if(msg[0]=='\0')
            break;

        char sendBuffer[129];
        for(i=0; i<numGroup; i++) {
            int pos = contacts[i]-1;
            if(pos>=messenger->numConn)
                continue;
            CONNECTION *conn = messenger_conn_getConnByPos(messenger, pos);

            // Send
            int msgSize = messenger_msg_encode(MSGTYPE_MSG, msg, strlen(msg), sendBuffer);
            send(conn->socket, sendBuffer, msgSize, 0);
        }

    }

    printf(">> Messages sent.\n");
}

void messenger_menu_checkMessages(MESSENGER *messenger) {
    printf("################# New messages #################\n");

    // Check connection list
    int i;
    int hasMessages = 0;
    for(i=0; i<messenger->numConn; i++) {
        CONNECTION *conn = messenger_conn_getConnByPos(messenger, i);

        // Check connection messages
        while(connection_hasMessages(conn)) {
            hasMessages = 1;

            // Pop message
            char msg[1024];
            time_t time;
            connection_popMessage(conn, msg, &time);

            // Convert time to str
            struct tm *timeinfo = localtime(&time);
            char timeStr[20];
            strftime(timeStr, 20,"[%d/%m/%y %Hh%M]", timeinfo);

            // Print
            printf("%s %s (%s): %s\n", timeStr, conn->username, conn->ip, msg);
        }
    }

    // No messages
    if(!hasMessages)
        printf("%s, you don't have new messages.\n", messenger->username);
}

int messenger_conn_connected2(MESSENGER *messenger, char ip[]) {
    // Check connection list
    int i;
    for(i=0; i<messenger->numConn; i++)
        if(strcmp(messenger->conn[i]->ip, ip)==0)
            return 1;
    return 0;
}

CONNECTION* messenger_conn_getConnByIP(MESSENGER *messenger, char ip[]) {
    // Search on conn list, and return
    int i;
    for(i=0; i<messenger->numConn; i++)
        if(strcmp(messenger->conn[i]->ip, ip)==0)
            return messenger->conn[i];
    return NULL;
}

int messenger_conn_getConnPosByIP(MESSENGER *messenger, char ip[]) {
    // Search on conn list, and return
    int i;
    for(i=0; i<messenger->numConn; i++)
        if(strcmp(messenger->conn[i]->ip, ip)==0)
            return i;
    return -1;
}

CONNECTION* messenger_conn_getConnByPos(MESSENGER *messenger, int pos) {
    // Check pos
    if(pos>=messenger->numConn)
        return NULL;

    // Return conn
    return messenger->conn[pos];
}

void messenger_conn_add(MESSENGER *messenger, CONNECTION *conn) {
    const int newListSize = messenger->numConn + 1;
    const int newPos = messenger->numConn;

    // Realloc list
    messenger->conn = realloc(messenger->conn, newListSize*sizeof(CONNECTION*));

    // Add element
    messenger->conn[newPos] = conn;

    // Inc counter
    (messenger->numConn)++;
}

void messenger_conn_remove(MESSENGER *messenger, int pos) {
    // Check size
    if(messenger->numConn==0)
        return;

    // Free connection
    free(messenger->conn[pos]);

    // Shift list and realloc
    int newListSize = messenger->numConn - 1;
    int i=0;
    for(i=pos; i<messenger->numConn-1; i++)
        messenger->conn[i] = messenger->conn[i+1];
    if(newListSize==0) {
        free(messenger->conn);
        messenger->conn = NULL;
    } else {
        messenger->conn = realloc(messenger->conn, newListSize*sizeof(CONNECTION*));
    }

    // Update counter
    messenger->numConn = newListSize;
}

int messenger_msg_encode(char msgType, char *data, int size, char dest[]) {
    // 1o byte = msg type
    dest[0] = msgType;
    // 2+ byte = data
    memcpy(dest+1, data, size);

    return size+1;
}
