#include "client.h"
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