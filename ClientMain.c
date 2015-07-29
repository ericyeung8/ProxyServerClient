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
#include "CFunctions.c"


int main(int argc, char **argv){

  int csd;
  struct sockaddr_in addr;
  struct hostent *hret;
  short port = 0;
  char buf[8192];
  int n_bytes = 0;
  char host[256];
  char page[256];
//  int iplen = 15;
//  char *ip = (char *)malloc(iplen+1);
  char *get;
  
  //int htmlstart = 0;
//  char * htmlcontent;
  
  fd_set ReadFds;
  fd_set connectedFds;

  int s_temp = 0;
  int maxfd = 0;

  //struct timeval tv;
  //tv.tv_sec = 10;
  //tv.tv_usec = 0;

/*  if(argc == 1){*/
/*    fprintf(stderr, "USAGE: htmlget host [page]\n\*/
/*	\thost: the website hostname. ex: coding.debuntu.org\n\*/
/*	\tpage: the page to retrieve. ex: index.html, default: /\n");*/
/*    exit(EXIT_FAILURE);*/
/*  }*/

/*  host = argv[1];*/
/*  if(argc > 2){*/
/*    page = argv[2];*/
/*  }*/
/*  else {*/
/*  page =PAGE;*/
/*  }*/

  //page = PAGE;


  //initialize the socket
  if ((csd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Cannot create client socket");
    exit(EXIT_FAILURE);
  }

  port = atoi(argv[2]);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  
  if ((hret = gethostbyname(argv[1])) == NULL){
    perror("Cannot get server's address");
    exit(EXIT_FAILURE);
  }

  memcpy(&addr.sin_addr.s_addr, hret->h_addr, hret->h_length);
  
  if (connect(csd,(struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1){
    perror("Cannot connect to Server");
    exit(EXIT_FAILURE);
  }

  memset(buf, 0, sizeof(buf));
//  buf = &argv[3];
  if (initialize(csd, argv[3]) != 1) {
    perror("ERROR!\n");
    exit(EXIT_FAILURE);
  }
  
  //initialize the select()
  FD_ZERO(&connectedFds);
  FD_SET(STDIN, &connectedFds);
  FD_SET(csd, &connectedFds);
  ReadFds = connectedFds;

  if (STDIN < csd) maxfd = csd;
    else maxfd = STDIN; 



  //loop: creat process to continue if the user doesn't type anything
  while(1){
    memset(buf, 0, sizeof(buf));

    ReadFds = connectedFds;

    s_temp = select(maxfd+1, &ReadFds, NULL, NULL, NULL);
    if (s_temp == -1){
      perror("select error!");
      exit(EXIT_FAILURE);
    }
    
    int i;
    int j;
    for (i=0; i <= maxfd; ++i){
      if ((j = FD_ISSET(i, &ReadFds)) == 1){
        if (i == STDIN){
        
/*          if ((hret = gethostbyname(argv[1])) == NULL){*/
/*            perror("Cannot get server's address");*/
/*            exit(EXIT_FAILURE);*/
/*          }*/
/*          */
/*          if(inet_ntop(AF_INET, (void *)hret->h_addr_list[0], ip, iplen) == NULL) {*/
/*            perror("Can't resolve host");*/
/*            exit(1);*/
/*          }*/
/*  */
/*          printf("IP is: %s\n", ip);*/
    
/*          memcpy(&addr.sin_addr.s_addr, hret->h_addr, hret->h_length);*/
  
/*          if (connect(csd,(struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1){*/
/*            perror("Cannot connect to Server");*/
/*            exit(EXIT_FAILURE);*/
/*          }*/
          
          memset(buf, 0, sizeof(buf));
/*          htmlstart = 0;*/
          n_bytes = read(0, buf, sizeof(buf));
          int k = 0;
          char *tran = &buf[0];
          
          if (strstr(buf, "http://") == NULL) {
            while(buf[k] != '/'){
              k++;
              if (buf[k] == '\n')
                break;
            }
          } else {
              while(buf[k+7] != '/'){
              k++;
              if (buf[k+7] == '\n')
                break;
            }
            tran += 7;
          }
          
          memset(host, 0, strlen(host));
          memcpy(host, tran, strlen(tran));
          host[k]='\0';
          
          memset(page, 0, strlen(page));
          page[0]='/';
          memcpy(page+1, tran+k+1, strlen(tran)-k-1);
          page[strlen(tran)-k-1] = '\0';
          
          get = build_get(host, page);
          fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
          
          if (send_get(csd, get) != 1) {
            perror("Send ERROR!\n");
            exit(EXIT_FAILURE);
          }
        }
      else if (i == csd){
        memset(buf, 0, sizeof(buf));
        n_bytes = 0;
        //htmlstart = 0;
        
        n_bytes = recv(csd, buf, sizeof(buf), 0);
        
        if (n_bytes < 0){
          perror("Receive ERROR!\n");
          exit(EXIT_FAILURE);
        }
        
        fprintf(stdout, "%s\n###############################################################\n", buf);
/*        if (n_bytes > 0){*/
/*          if (htmlstart == 0){*/
/*            //htmlcontent = strstr(buf, "\r\n\r\n");*/
/*            htmlcontent = buf;*/
/*            if (htmlcontent != NULL){*/
/*              htmlstart = 1;*/
/*              //htmlcontent += 4;*/
/*            }*/
/*          }*/
/*          else {*/
/*            htmlcontent = buf;*/
/*          }*/
/*        }*/
/*        */
/*        if (htmlstart == 1) {*/
/*          fprintf(stdout, "%s", htmlcontent);*/
/*        }*/
        
      }  //end if i == csd
      }  //end read and send
    } //end search fd loop
  } //end main loop

  return 0;
}
