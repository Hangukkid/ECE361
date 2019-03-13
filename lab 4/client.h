#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int checkValidInput (char *message) {
    return 0;
}

void *get_in_addr (struct sockaddr *sa)
{
    if (sa -> sa_family == AF_INET) return &(((struct sockaddr_in*) sa) -> sin_addr);
    return &(((struct sockaddr_in6*) sa) -> sin6_addr);
}

#endif