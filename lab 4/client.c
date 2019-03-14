#include "client.h"

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

void client_to_server (int sockfd, char *username) {
    char buffer[100] = "";

    // username
    if (send(sockfd, username, strlen(username), 0) == -1) {
        perror("send");
        exit(1);
    }  

    while (1) {
        printf("input: ");
        scanf("%[^\n]%*c", buffer);
        if(recv(sockfd, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == 0) {
            printf("server has unexpectedly closed.\n");
            return;
        }
        if (send(sockfd, remove_whitespace(buffer), 100, 0) == -1) {
            perror("send");
            exit(1);
        }  
    }

}

int main(int argc, char *argv[])
{
    int sockfd;  

    if (argc != 4) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    sockfd = setupClient(argv[1], argv[2]);
    client_to_server(sockfd, argv[3]);
    close(sockfd);

    return 0;
}