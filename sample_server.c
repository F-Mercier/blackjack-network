#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT "5000"
#define BACKLOG 20
#define MAXDATASIZE 100

void* run_thread(void* sockfd){
  int sfd = *((int*)sockfd);

  char* init_msg = "req: pseudo";
  if(send(sfd,init_msg,strlen(init_msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }

  char readbuf[MAXDATASIZE];

  int numbytes;

  if((numbytes = recv(sfd,readbuf,MAXDATASIZE-1,0)) == -1){
    fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
    exit(1);
  }

  printf("numbytes =  %d\n", numbytes);
  readbuf[numbytes] = '\0';

  printf("connected client has pseudo : %s \n",readbuf);
  
  pthread_exit(NULL);
}


int main(int argc, char** argv){
  int sockfd, new_sockfd;
  struct addrinfo hints, *serverinfo, *p;
  struct sockaddr_storage* client_addrs;
  socklen_t sin_size;
  int yes = 1;
  char ip[INET6_ADDRSTRLEN];
  int status;

  if(argc != 2){
    fprintf(stderr, "%s: usage: sample_server max_player_on_table\n", argv[0]);
    return 1;
  }


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if(( status = getaddrinfo(NULL,PORT,&hints,&serverinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(status));
    return 1;
  }

  for(p = serverinfo; p!=NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      fprintf(stderr, "socket: unable to create socket : %s\n", strerror(errno));
      continue;
    }
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      fprintf(stderr, "setsockopt error\n");
      exit(1);
    }
    
    if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      fprintf(stderr, "bind: unable to bind to the address : %s\n",strerror(errno));
      continue;
    }
    break;
  }

  freeaddrinfo(serverinfo);

  if(p == NULL){
    fprintf(stderr, "%s: failed to bind \n", argv[0]);
    exit(1);
  }

  if(listen(sockfd,BACKLOG) == -1){
    fprintf(stderr, "listen: unable to listen : %s\n", strerror(errno));
    exit(1);
  }

  printf("listening\n");
  int t = 0;
  int rc;
 
  while(1){
    sin_size = sizeof(client_addrs);
    new_sockfd = accept(sockfd,(struct sockaddr*) client_addrs, &sin_size);
    if(new_sockfd < 0){
      fprintf(stderr, "accept: error while accepting: %s\n",strerror(errno));
      continue;
    }

    pthread_t thread;
    pthread_attr_t att;
    pthread_attr_init(&att);

    printf("accepted\n");

    rc = pthread_create(&thread,&att, run_thread , &new_sockfd);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
    
    t++;

  }

  pthread_exit(NULL);
  return 0;
}
  
