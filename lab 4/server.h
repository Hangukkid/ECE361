#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

// #define SERVERPORT "4950"    // the port users will be connecting to
#define IPADD "127.0.0.1"
#define BACKLOG 10

void sigchld_handler (int s) 
{
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr (struct sockaddr *sa)
{
    if (sa -> sa_family == AF_INET) return &(((struct sockaddr_in*) sa) -> sin_addr);
    return &(((struct sockaddr_in6*) sa) -> sin6_addr);
}

#endif