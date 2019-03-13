#include "client.h"

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, connected;
    int numbytes, received;
    char s[INET6_ADDRSTRLEN];
    // server_add: argv[1], serverport: argv[2], username: argv[3]

    if (argc != 4) {
        printf("Not enough arguments\n");
        return 0;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p -> ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        if (connect(sockfd, p -> ai_addr, p -> ai_addrlen) == -1) {
            close (sockfd);
            perror ("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    char message[100] = "";
    char echo[100] = "";

    
    freeaddrinfo(servinfo);
    while (1) {  
        printf("Your message: ");
        scanf ("%s", message);
        printf("\n");
        inet_ntop(p -> ai_family, get_in_addr((struct sockaddr *) p -> ai_addr), s, sizeof s);
        if ((numbytes = send(sockfd, message, strlen(message), 0)) == -1) {
            perror("talker: sendto");
            exit(1);
        }

        // if ((received = recv(sockfd, echo, 100, 0)) == -1) {
        //     perror("talker: receivefrom");
        //     exit(1);
        // }
    }
    
    close(sockfd);

    return 0;
}