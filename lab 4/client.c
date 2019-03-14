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

int expecting_response (char *buf) {
    char list[] = "list";
    char exits[] = "exit";
    if (!strcmp (buf, list)) return 1;
    if (!strcmp (buf, exits)) return 2;
    
    return 0; 
}

void client_to_server (int sockfd, char *username) {
    char buf[100] = "";
    int numbytes;

    // username
    if (send(sockfd, username, strlen(username), 0) == -1) {
        perror("send");
        exit(1);
    }  

    while (1) {
        printf("input: ");
        scanf(" %[^\n]%*c", buf);   
        if(recv(sockfd, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0) {
            printf("server has unexpectedly closed.\n");
            return;
        }
        int expecting = expecting_response (remove_whitespace(buf));
        if (send(sockfd, remove_whitespace(buf), 100, 0) == -1) {
            perror("send");
            exit(1);
        }  
        // reset buffer
        memset(buf, 0, 100);  
        if (expecting) {
            if ((numbytes = recv(sockfd, buf, 100, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            if (expecting == 1) {
                printf ("%s", buf);
            } else {
                printf ("Exiting Client.\n");
                exit(1);
            }
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