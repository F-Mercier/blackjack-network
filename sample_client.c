#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "5000"
#define MAXDATASIZE 100

void* get_in_addr(struct sockaddr* sa){
  if(sa->sa_family == AF_INET){
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char** argv){
  int sockfd, numbytes;
  char rbuf[MAXDATASIZE];
  char sbuf[MAXDATASIZE];
  struct addrinfo hints, *serverinfo, *p;
  int rv;
  char serverip[INET6_ADDRSTRLEN];
  char pseudo[20];

  if(argc != 3){
    fprintf(stderr,"usage: client hostname pseudo\n");
    exit(1);
  }

  printf("%s\n",argv[2]);
  strncpy(pseudo,argv[2],19);
  pseudo[19]='\0';

  printf("your pseudo is : %s\n",pseudo);
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if( (rv=getaddrinfo(argv[1],PORT,&hints,&serverinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));
    return 1;
  }

  for(p = serverinfo; p!=NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      fprintf(stderr, "socket: unable to create socket: %s\n",strerror(errno));
      continue;
    }
    
    if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      fprintf(stderr, "bind: unable to bind to the address: %s\n",strerror(errno));
      continue;
    }
    break;
  }

  if(p==NULL){
    fprintf(stderr, "%s: failed to connect\n",argv[0]);
    exit(1);
  }

  inet_ntop(p->ai_family,get_in_addr((struct sockaddr*)p->ai_addr),serverip,sizeof(serverip));
  printf("%s: connected to %s\n",argv[0],serverip);

  freeaddrinfo(serverinfo);

  if((numbytes = recv(sockfd,rbuf,MAXDATASIZE-1,0)) == -1){
    fprintf(stderr,"recv : error while reading from the server : %s\n",strerror(errno));
    exit(1);
  }

  rbuf[numbytes] = '\0';

  printf("%s : received '%s'\n",argv[0],rbuf);

  if(send(sockfd,pseudo,strlen(pseudo),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
  printf("finish sending\n");

  return 0;
}
  
  

