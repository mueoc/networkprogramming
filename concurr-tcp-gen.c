#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h> // För sockaddr_in
#include <sys/socket.h> // För socket-funktioner
#include <arpa/inet.h>  // För inet_pton, inet_ntoa m.m.
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h> // MAC inet_ntop
/* You will to add includes here */


// Included to get the support library
#include <calcLib.h>

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG


using namespace std;


void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
  switch(sa->sa_family)
  {
    case AF_INET:
        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
          s, maxlen);
        break;

    case AF_INET6:
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
          s, maxlen);
        break;

    default:
      strncpy(s, "Unknown AF", maxlen);
      return NULL;
  }

  return s;
}

int main(int argc, char *argv[]){
  if (argc < 2) {
    fprintf(stderr, "Usage: %s protocol://server:port/path.\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  char *input = argv[1];
  char *sep = strchr(input, ':');
  
  if (!sep) {
    fprintf(stderr, "Error: input must be in host:port format\n");
    return 1;
  }
  
  // Allocate buffers big enough
  char hoststring[256];
  char portstring[64];
  
  // Copy host part
  size_t hostlen = sep - input;
  if (hostlen >= sizeof(hoststring)) {
    fprintf(stderr, "Error: hostname too long\n");
    return 1;
  }
  strncpy(hoststring, input, hostlen);
  hoststring[hostlen] = '\0';
  
  // Copy port part
  strncpy(portstring, sep + 1, sizeof(portstring) - 1);
  portstring[sizeof(portstring) - 1] = '\0';
  
  printf("TCP server on: %s:%s\n", hoststring,portstring);

  int sockfd, connfd, len, childCount, pid, n;
  struct sockaddr_in servaddr, cliaddr;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("socket creation failed...\n");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(portstring));
  if((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0){
    perror("socket bind failed...\n");
    exit(0);
  }

  if((listen(sockfd, 5)) != 0){
    perror("Listen failed...\n");
    exit(0);
  }

  while(1) {
    std::cout << "boring 3 " << std::endl;

    memset(&cliaddr, 0, sizeof(cliaddr));
    len = sizeof(cliaddr);
    if((connfd = accept(sockfd, (struct sockaddr*)&cliaddr, (socklen_t*)&len))<0){
      perror("server accept failed...\n");
      exit(0);
    }
    childCount++;
    std::cout << "boring 4 " << std::endl;


    printf("listener: got packet from %s:%d\n", inet_ntop(cliaddr.sin_family,
       get_in_addr((struct sockaddr*)&cliaddr), hoststring, sizeof(hoststring)), ntohs(cliaddr.sin_port));
       
    pid = fork();
    if (pid == 0) {
      get_ip_str((struct sockaddr*)&cliaddr, hoststring, sizeof(hoststring));
      std::cout << "Child " << childCount << " handling client " << hoststring << ":" << ntohs(cliaddr.sin_port) << std::endl;
      char bufferText[1024];
      while(1)
      {
        n = send(connfd,bufferText,strlen(bufferText),0);
        if (n < 0) {
          perror("ERROR writing to socket");
          exit(1);
        }
        std::cout << "Child " << childCount << " waiting for input from client " << hoststring << ":" << ntohs(cliaddr.sin_port) << std::endl;
        sleep(1);
      }
      exit(0);
    }
    else 
    {
      close(connfd);
    }
    close(connfd);
  }
  return 0;
}
