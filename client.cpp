/*
** client.c -- a stream socket client demo
*/

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

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 


char buf[MAXDATASIZE];

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int send_to_server(char str_to_send[],int sockfd)
{
//fgets(buf,100,stdin)  ;
strcpy(buf,str_to_send);
int numbytes;
    printf("client send: %s\n", buf);
if(send(sockfd, strtok(buf,"\n"), MAXDATASIZE, 0)==-1)
                perror("error"); 
 
    memset(buf, 0, MAXDATASIZE);
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
        perror("error");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client received: '%s', %d\n",buf,numbytes);
    if(strcmp("Code94",strtok(buf,":"))==0)
return -1;
else return 1;
}


int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
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

//scanf("%s",buf);
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

send_to_server("login votming",sockfd);
send_to_server("password 123",sockfd); 
int result=1;
while(result==1){  
result*=send_to_server("calc 1+1",sockfd); 
result*=send_to_server("calc 20+25-12*236*0.12",sockfd); 
result*=send_to_server("calc 46*0.754/44.256",sockfd);  
}
    close(sockfd);

    return 0;
}
