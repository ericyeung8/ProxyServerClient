#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "Common.h"
#include "SFunctions.c"
#include "CacheFunction.c"


int main(int argc, char** argv){
/*check the input*/

  int max_fd = 0;
  int servSocket = 0;
  int newFd = 0;
  int n_bytes = 0;
  char buf[256];
  
  int i;
  fd_set readFds;
  fd_set connectedFds;
  fd_set writeFds;
  FD_ZERO(&readFds);
  FD_ZERO(&connectedFds);
  FD_ZERO(&writeFds);
  
  struct addrinfo temp;
  struct addrinfo *res;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  
  memset(&temp, 0, sizeof(temp));
  temp.ai_family = AF_UNSPEC;
  temp.ai_socktype = SOCK_STREAM;
  
  int addrVal;
  
  if ((addrVal = getaddrinfo(argv[1], argv[2], &temp, &res)) != 0) {
    perror("getaddrinfo() error: ");
    exit(EXIT_FAILURE);
  }
  
  servSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (servSocket == -1) {
    perror("socket() error: ");
    exit(EXIT_FAILURE);
  }
  
  int yes = 1;
  if (setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt() error: ");
    exit(EXIT_FAILURE);
  }
  
  if (bind(servSocket, res->ai_addr, res->ai_addrlen) == -1){
    perror("bind() error: ");
    close(servSocket);
    exit(EXIT_FAILURE);
  }
  
  freeaddrinfo(res);
  
  printf("Server started successfully!\n");
  
  struct sockaddr_in tempaddr;
  unsigned int templen = sizeof(struct sockaddr);
  getsockname(servSocket, (struct sockaddr*)&tempaddr, &templen);
  
  if (listen(servSocket, 10) == -1){
    perror("listen() error: ");
    exit(EXIT_FAILURE);
  }
  printf("Server listens to port: %d\n", ntohs(tempaddr.sin_port));
  
  FD_SET(servSocket, &connectedFds);
  max_fd = servSocket;
  
  while(1){
    memset(buf, 0, sizeof(buf));
    FD_ZERO(&readFds);
    readFds = connectedFds;
    
    if (select(max_fd+1, &readFds, NULL, NULL, NULL) == -1){
      perror("select() error: ");
      exit(EXIT_FAILURE);
    }
    
    for (i=0; i<=max_fd; ++i) {
      if (FD_ISSET(i, &readFds)) {
        if (i == servSocket){
          addrlen = sizeof remoteaddr;
          newFd = accept(servSocket, (struct sockaddr *)&remoteaddr, &addrlen);
          if (newFd == -1){
            perror("accept() error: ");
          } else if (newFd < (atoi(argv[3])+servSocket+1)) {//Determine whether more than the maximum number of users
              n_bytes = recv(newFd, buf, sizeof(buf), 0);
              if (n_bytes != 0){//deal with get from client
                HandleMsg(newFd, buf);
                FD_SET(newFd, &connectedFds);
                if (newFd > max_fd)	max_fd = newFd;
              }
            } else {
                close(newFd);
              }
        } else {
            if ((n_bytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
              if (n_bytes == 0) {
                printf("selectserver: socket %d hung up\n", i);
              } else {
                  perror("recv() error: ");
                }
              close(i);
              
              FD_CLR(i, &connectedFds);
            } else {
                //handle data from webserver
                HandleMsg(newFd, buf);
                //fprintf(stdout, "%s", buf);
              }
          }
      }
    }
  }
  
  return 0;
}
