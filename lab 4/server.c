#include "server.h"

char *client_to_string(client *c) {
    char beginning[] = "\t";
    char ending[] = "\n";
    char *client_string = (char *) malloc(sizeof(char) * (strlen(c -> username) + 10));
    c -> username;
    strcpy (client_string, beginning);
    strcat(client_string, c -> username);
    strcat(client_string, ending);

    return client_string;
}

int find_blankspace (char *buf) {
    for (int i = 0; i < strlen(buf); i++) {
        if (buf[i] == ' ') {
            return i;
        }
    }    
    return -1;
}

void send_message_to_client (client *from, client *to, char *message) {
    char full_message[300] = "";
    char format[] = " says: ";
    char ending[] = "\n";
    if (from != NULL && to != NULL) {
        strcpy (full_message, from -> username);
        strcat (full_message, format);
        strcat (full_message, message);
        strcat (full_message, ending);
        // printf("full message: %s\n", full_message);
        if (send(to -> sockfd, full_message, 300, 0) == -1) {
            perror("send");
            exit(1);
        }        
    }
}

void send_error_message_to_client (client *to, int error) {
    char already_exists[] = "Username already exists!\n";
    switch (error) {
        case 0:
            if (send(to -> sockfd, already_exists, strlen(already_exists), 0) == -1) {
                perror("send");
                pthread_exit(NULL);
            }
            break;
        default:
            break;
    }
}

void *handle_client (void *cl) {
    char buf[100] = "";
    char list[] = "list";
    char exits[] = "exit";
    int numbytes;
    client *curr_client = (client *) cl;
    
    while (1) {
        // check if client has disconnected
        if (recv(curr_client -> sockfd, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0) {
            printf("%s has abruptly terminated the connection\n", curr_client -> username);
            client_logout (curr_client -> sockfd);
            pthread_exit(NULL);
        }
        // receive input from client
        if ((numbytes = recv(curr_client -> sockfd, buf, 100, 0)) == -1) {
            printf("%s has abruptly terminated the connection\n", curr_client -> username);
            client_logout (curr_client -> sockfd);
            pthread_exit(NULL);
        }
        // printf ("Number of bytes: %d\n", numbytes);
        printf("Received from user: '%s'.\n", buf);

        if (buf[0] == 'b' && buf[1] == 'r' && buf[2] == 'o' && 
            buf[3] == 'a' && buf[4] == 'd' && buf[5] == 'c' &&
            buf[6] == 'a' && buf[7] == 's' && buf[8] == 't') {
            // printf("broadcast.\n");
            if (strlen(buf) > 10) {
                char *message = strdup(buf + 10);
                pthread_mutex_lock(&server_lock);
                for (client *c = m -> all_clients; c != NULL; c = c -> next) {
                    if (c != curr_client)
                        send_message_to_client (curr_client, c, message);
                }
                pthread_mutex_unlock(&server_lock);
                // printf("message: %s\n", message);
            }
        } else if (!strcmp (buf, list)) {
            char users_list[3000] = "List of users:\n"; 

            pthread_mutex_lock(&server_lock);
            for (client *c = m -> all_clients; c != NULL; c = c -> next) {
                strcat(users_list, client_to_string(c));
            }
            pthread_mutex_unlock(&server_lock);

            printf("%s", users_list);
            if (send(curr_client -> sockfd, users_list, 3000, 0) == -1) {
                perror("send");
                pthread_exit(NULL);
            }
        } else if (!strcmp (buf, exits)) {
            printf("Goodbye %s!\n", curr_client -> username);
            client_logout (curr_client -> sockfd);
            if (send(curr_client -> sockfd, buf, 100, 0) == -1) {
                perror("send");
                pthread_exit(NULL);
            }
            pthread_exit(NULL);
        } else {
            int i = find_blankspace(buf);
            char *message, *username;
            if (i > 0) {
                message = strdup(buf + i + 1);
                username = strndup(buf, i);
                printf("message: %s, username: %s \n", message, username);
                client *recipient = find_client (username);
                if (recipient != NULL) {
                    send_message_to_client (curr_client, recipient, message);
                }
            }
        }
        // reset buffer
        memset(buf, 0, 100);    
    }
}

int client_login (client *c) {
    char buffer[100] = "";
    if (recv(c -> sockfd, buffer, 100, 0) == 0) {
        perror("username recv");
        exit(1);
    }
    c -> username = (char *) malloc(sizeof(char) * strlen(buffer));
    strcpy (c -> username, buffer);

    if (find_client (c -> username) != NULL) {
        free (c -> username);
        return 0;
    } 
    pthread_mutex_lock(&server_lock);
    c -> next = m -> all_clients;
    m -> all_clients = c;
    m -> num_connections += 1;
    pthread_mutex_unlock(&server_lock);
    return 1;
}

void client_logout (int sockfd) {
    // if first one
    pthread_mutex_lock(&server_lock);
    if (m -> all_clients -> sockfd == sockfd) {
        client *remove_it = m -> all_clients;
        m -> all_clients = remove_it -> next;
        free (remove_it -> username);
        close (remove_it -> sockfd);
        pthread_mutex_unlock(&server_lock);
        return;
    }
    client *lagger = m -> all_clients;
    for (client *c = m -> all_clients; c != NULL; c = c -> next) {
        if (c -> sockfd == sockfd) {
            lagger -> next = c -> next;
            free (c -> username);
            close (c -> sockfd);
            pthread_mutex_unlock(&server_lock);
            return;
        }
        lagger = c;
    }    
    pthread_mutex_unlock(&server_lock);
}

client *find_client (char *username) {
    pthread_mutex_lock(&server_lock);
    for (client *c = m -> all_clients; c != NULL; c = c -> next) {
        if (!strcmp(c -> username, username)) {
            pthread_mutex_unlock(&server_lock);
            return c;
        }
    }
    pthread_mutex_unlock(&server_lock);
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
        // client_login (new_client);
        if (!client_login (new_client)) {
            printf("shit\n");
            send_error_message_to_client(new_client, 0);
            close(new_client -> sockfd);
            free (new_client);
            continue;
        }
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        printf("server: got connection from %s\n", new_client -> username);

        pthread_create(&(new_client -> thread), NULL, handle_client, (void *) new_client);
    }
    close(sockfd); // child doesn't need the listener
    return 0;
}