// #include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

// #define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int setupClient (char *ip_add, char *portno) {
    int sockfd;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip_add, portno, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
}
#include <pthread.h>

// typedef struct {
//     int sockfd;
//     char *username;
// } client;

int exited = 0;

char *remove_whitespace (const char *string) {
    while(isspace((unsigned char)string[0]))
        string++;
    char *final = strdup(string);
    int length = strlen(final);
    while(length > 0 && isspace((unsigned char)final[length-1]))
        length--;
    final[length] = '\0';
    return final;
}

void input_characters (char *buf, int size) {
    int i = 0;
    int ch;
    while((ch = getchar()) != '\n' && ch != EOF ) {
        if (i < size) {
            buf[i++] = ch;
        }
    }
    buf[i] = '\0';
}

void client_to_server (int sockfd, char *username) {
    char buf[100] = "";
    int numbytes;
    char exits[] = "exit";

    // username
    if (send(sockfd, username, strlen(username), 0) == -1) {
        perror("send");
        exited = 1;
        exit(1);
    }  

    while (1) { 
        input_characters (buf, sizeof(buf)); 
        if(recv(sockfd, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0) {
            printf("server has unexpectedly closed.\n");
            exited = 1;
            return;
        }
        // int expecting = expecting_response (remove_whitespace(buf));
        if (send(sockfd, remove_whitespace(buf), strlen(buf), 0) == -1) {
            perror("send");
            exited = 1;
            exit(1);
        } 
        if (!strcmp(exits, remove_whitespace(buf))) {
            exited = 1;
            printf("Client has gracefully closed.\n");
            exit(1);
        }

    }
}

void *server_to_client (void *sock) {
    char buf[100] = "";
    int numbytes;
    int *sockfd = ((int *)sock);
    char already_received[] = "Username already exists!\n";

    while (1) {  
        if (!strcmp(buf, already_received)) {
            exit(1);
        }
        if(recv(*sockfd, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0) {
            printf("server has unexpectedly closed.\n");
            exit(1);
        }
        if ((numbytes = recv(*sockfd, buf, 100, 0)) == -1) {
            perror("recv");
            exit(1);
        }  
        // reset buffer
        if (strlen(buf) != 0) {
            printf ("%s", buf);
            memset(buf, 0, 100);
        }  
        if (exited) {
            exit(1);
        }
        fflush(stdout);
    }
}

int main(int argc, char *argv[])
{
    int sockfd;  
    // pthread_t client_server;
    pthread_t server_client;

    if (argc != 4) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    sockfd = setupClient(argv[1], argv[2]);
    if (pthread_create (&server_client, NULL, &server_to_client, &sockfd)) {
        fprintf (stderr, "Error in creating thread\n");
        return 1;
    }

    client_to_server(sockfd, argv[3]);
    close(sockfd);

    return 0;
}