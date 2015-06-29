#ifndef CLIENT_H
#define CLIENT_H

#include "global.h"

int client_connect(char *ip, int port);
int client_disconnect(int sock);

#endif // CLIENT_H
