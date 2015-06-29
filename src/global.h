#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>

void socket2ip(int socket, char retn[17]);

#endif // GLOBAL_H
