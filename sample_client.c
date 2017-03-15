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

#include "client_game.h"

#define PORT "5001"
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


  //############################################
  //the game begins here..........................
  //###############################################

  
  game_instance* game = init_game(sockfd,pseudo);

  printf("game initialized\n");

  char msg[2*MAXDATASIZE];
  memset(msg,0,2*MAXDATASIZE);
  //main loop for the client
  while(1){
    //system("clear");
    //at each step in the loop check connectivity
    message m = get_message(sockfd,msg,2*MAXDATASIZE);

    if(m == player_disconnected){
      char ppseudo[20];
      memset(ppseudo,0,20);
      int k=0;
      for(int j = 20; j<strlen(msg);j++){
	ppseudo[k] = msg[j];
	k++;
      }
      ppseudo[k] = '\0';
      for(int i = 0; i< game->number_of_players; i++){
	if(strncmp(game->players_pseudos[i],ppseudo,strlen(ppseudo))==0){
	  game->players_connected[i] = 0;
	  break;
	}
      }
    }
    if(m == first_card){
      char card[10];
      memset(card,0,10);
      int i = 11;
      int k = 0;
      while(msg[i] != '('){
	card[k] = msg[i];
	i++;
	k++;
      }
      i++;//skip (
      card[k] = '\0';
      char pseudonim[20];
      memset(pseudonim,0,20);
      int l = 0;
      while(msg[i] != ')'){
	pseudonim[l] = msg[i];
	i++;
	k++;
      }
      pseudonim[l] = '\0';
      
      card_t* fcard = string_to_card(card);//to implement
      add_card_to_hand(game,fcard,pseudonim);// to implement
    }
    if(m == req_connected){
      send_keep_connection(sockfd);
    }

    printf("before printing the game\n\n");
    print_game(game);
    
  }

  return 0;
}
  
  

