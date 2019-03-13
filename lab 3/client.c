/*
** talker.c -- a datagram "client" demo
*/

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
#include <sys/time.h>

#define SERVERPORT "4950"    // the port users will be connecting to
#define IPADD "127.0.0.1"


double computeR (int num_bytes_1, int num_bytes_2, double t0_1, double t0_2) {
    int diff_bits = (num_bytes_2 - num_bytes_1)*8;
    double diff_t0 = (t0_2 - t0_1)/2;
    return diff_bits/diff_t0; // per second instead of microsecond
}

double computeTprop (int num_bytes, double R, double t0) {
    return (t0/2 - num_bytes*8/R); 
}

void computeAnswers (double *t0, int size) {
    double Rave = 0;
    double tpropave = 0;
    for (int num_bytes = 1; num_bytes < size; num_bytes++) {
        double R = computeR(num_bytes, num_bytes + 1, t0[num_bytes - 1], t0[num_bytes]);
        double tprop = computeTprop(num_bytes, R, t0[num_bytes - 1]);
        Rave += R;
        tpropave += tprop;
        printf("R = %f, \t tprop = %f\n", R*1000000, tprop/1000000);
    }
    Rave = Rave/(size-1)*1000000;
    tpropave = (tpropave/(size-1))/1000000;
    printf("R average = %f, \t tprop average = %f\n", Rave, tpropave);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, connected;
    int numbytes, received;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(IPADD, SERVERPORT, &hints, &servinfo)) != 0) {
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

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    if (connected = connect(sockfd, servinfo -> ai_addr, servinfo -> ai_addrlen) == -1) {
        fprintf (stderr, "Connection Failed\n");
    }

    char message[100] = "o";
    char echo[100];
    struct timeval tvalBefore, tvalAfter; 

    unsigned count = 0;
    unsigned num_times_test = 3;
    unsigned max_num_bytes = 5;
    double t0 [max_num_bytes];
    while (count < max_num_bytes) {
        // printf("Input your message: ");
        // scanf ("%s", message);
        double total = 0;
        count++;
        for (int i = 0; i < num_times_test; i++) {
            gettimeofday (&tvalBefore, NULL);
            if ((numbytes = send(sockfd, message, strlen(message), 0)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
            // printf("talker: sent %d bytes to %s\n", numbytes, IPADD);

            if ((received = recv(sockfd, echo, 100, 0)) == -1) {
                perror("talker: receivefrom");
                exit(1);
            }
            gettimeofday (&tvalAfter, NULL);
            total += (tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec - tvalBefore.tv_usec;
        }
        // printf ("%d sends to simulator for %d bits takes on average %f microseconds\n", num_times_test, strlen(echo) * 8, total/num_times_test);
        message[count] = 'o';
        t0[count - 1] = total/num_times_test;
    }
    computeAnswers(t0, max_num_bytes);
    freeaddrinfo(servinfo);
    close(sockfd);

    return 0;
}