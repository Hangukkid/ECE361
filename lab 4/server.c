#include "server.h"
#include <pthread.h>
#include <malloc.h>

void handleClient (client *new_client) {
    
}

void add_client (client *c) {
    c -> next = m -> all_clients;
    m -> all_clients = c;
    m -> num_connections += 1;
}

client *findClient (char *username) {
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;

    sockfd = setupServer(s, argv[1]);

    if (argc != 2) {
        fprintf(stderr,"usage: server port\n");
        exit(1);
    }   

    printf("server: waiting for connections...\n");

    char buffer[100] = "";

    while(1) {  // main accept() loop
        client *new_client = (client *) malloc (sizeof(client));
        sin_size = sizeof their_addr;
        new_client -> sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_client -> sockfd == -1) {
            perror("accept");
            continue;
        }
        add_client (new_client);
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if ((numbytes = recv(new_client -> sockfd, buffer, 100, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        printf ("Number of bytes: %d\n", numbytes);
        printf("Received from user: '%s'.\n", buffer);
        memset(buffer, 0, 100);
        // }
    }
    close(sockfd); // child doesn't need the listener
    return 0;
}