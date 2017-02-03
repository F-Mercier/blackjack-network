#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT "5000"
#define BACKLOG 20


int main(int argc, char** argv){
  int sockfd, new_sockfd;
  struct addrinfo hints, *serverinfo, *p;
  struct sockaddr_storage* client_addrs;
  socklen_t sin_size;
  int yes = 1;
  char ip[INET6_ADDRSTRLEN];
  int status;

  if(argc != 2){
    fprintf(stderr, "%s: usage: sample_server NR_MAX_PER_TABLE\n", argv[0]);
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
      fprintf(stderr, "socket: unable to create socket\n");
      continue;
    }
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      fprintf(stderr, "setsockopt error\n");
      exit(1);
    }
    
    if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      fprintf(stderr, "bind: unable to bind to the address\n");
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
    fprintf(stderr, "listen: unable to listen\n");
    exit(1);
  }

  int waiting_table = 0; // there is no table which waits for players either because all are full or no table whatsoever
  int players_no = 0;
  //int tables_pid[100];//maximum number of tables
  //int pid_ind = -1;// currently no table
  int pid;
  int mpipefd[2];
  int fpipefd[2];
  
  //for(int i=0; i<100;i++) tables_pid[i] = 0;// initalize the pid of every table to zero

  if(pipe(mpipefd) == -1){
    fprintf(stderr, "pipe: unable to create pipe\n");
    exit(1);
  }
  
  while(1){
    sin_size = sizeof(client_addrs);
    new_sockfd = accept(sockfd,(struct sockaddr*) client_addrs, sin_size);
    if(new_sockfd == -1){
      fprintf(stderr, "accept: error while accepting\n");
      continue;
    }

    
    


  }
  
  return 0;
}
  
