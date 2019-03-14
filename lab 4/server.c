#include "server.h"
#include <pthread.h>
#include <malloc.h>

char *client_to_string(client *c) {
    char ending[] = "\n";
    char *client_string = (char *) malloc(sizeof(char) * (strlen(c -> username) + 10));
    c -> username;
    strcpy(client_string, c -> username);
    strcat(client_string, ending);

    return client_string;
}


void handle_client (client *new_client) {
    char buf[100] = "";
    char list[] = "list";
    char exits[] = "exit";
    int numbytes;
    
    while (1) {
        // check if client has disconnected
        if (recv(new_client -> sockfd, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0) {
            printf("Client has closed the connection\n");
            return;
        }
        // receive input from client
        if ((numbytes = recv(new_client -> sockfd, buf, 100, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        printf ("Number of bytes: %d\n", numbytes);
        printf("Received from user: '%s'.\n", buf);

        if (buf[0] == 'b' && buf[1] == 'r' && buf[2] == 'o' && 
            buf[3] == 'a' && buf[4] == 'd' && buf[5] == 'c' &&
            buf[6] == 'a' && buf[7] == 's' && buf[8] == 't') {
            printf("broadcast.\n");
            if (strlen(buf) > 10) {
                char *message = strdup(buf + 10);
                printf("message: %s\n", message);
            }
        } else if (!strcmp (buf, list)) {
            char users_list[3000] = "List of users:\n"; 
            for (client *c = m -> all_clients; c != NULL; c = c -> next) {
                strcat(users_list, client_to_string(c));
            }
            if (send(new_client -> sockfd, users_list, 3000, 0) == -1) {
                perror("send");
                exit(1);
            }
        } else if (!strcmp (buf, exits)) {
            printf("Goodbye %s!\n", new_client -> username);
            if (send(new_client -> sockfd, buf, 100, 0) == -1) {
                perror("send");
                exit(1);
            }
            return;
        } else {

        }

        // reset buffer
        memset(buf, 0, 100);    
    }
}

void client_login (client *c) {
    char buffer[100] = "";
    if (recv(c -> sockfd, buffer, 100, 0) == 0) {
        perror("username recv");
        exit(1);
    }
    c -> username = (char *) malloc(sizeof(char) * strlen(buffer));
    strcpy (c -> username, buffer);

    c -> next = m -> all_clients;
    m -> all_clients = c;
    m -> num_connections += 1;
}

void client_logout (char *username) {

}

client *find_client (char *username) {
    for (client *c = m -> all_clients; c != NULL; c = c -> next) {
        if (!strcmp(c -> username, username)) {
            return c;
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd;  // listen on sock_fd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;

    sockfd = setupServer(s, argv[1]);

    if (argc != 2) {
        fprintf(stderr,"usage: server port\n");
        exit(1);
    }   

    printf("server: waiting for connections...\n");

    while(1) {  
        client *new_client = (client *) malloc (sizeof(client));
        sin_size = sizeof their_addr;
        new_client -> sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_client -> sockfd == -1) {
            perror("accept");
            continue;
        }
        client_login (new_client);
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        printf("server: got connection from %s\n", new_client -> username);

        handle_client(new_client);
    }
    close(sockfd); // child doesn't need the listener
    return 0;
}