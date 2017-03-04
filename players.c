#include "players.h"
#include "pseudos.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAXDATASIZE 100

player* init_player(int socket_fd, pseudo_db* pb){
  //printf("\ninside init_player()\n");
  player* p = (player*)malloc(sizeof(player));
  p->socket_fd = socket_fd; //set the socket descriptor
  p->connected = 1; //set connected to true
  // check if pseudo already exists before we bind it to the player
  //steps : check_existance - bind_pseudo - set pseudo to player
  char* pseudo;
  pseudo = ask_for_pseudo(socket_fd);
  if(check_existance(pb,pseudo) == 0){
    bind_pseudo(&pb,pseudo);
  }else{
    while(check_existance(pb,pseudo) != 0){
      printf("inside while()\n");
      pseudo = ask_for_pseudo(socket_fd);
    }
    bind_pseudo(&pb,pseudo);
  }
  p->pseudo = pseudo;
  return p;
}


char* ask_for_pseudo(int socket_fd){
  //printf("\ninside ask_pseudo()\n ");
  char* init_msg = "req: pseudo";
  
  if(send(socket_fd,init_msg,strlen(init_msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }

  char* readbuf = (char*) malloc(sizeof(char) * MAXDATASIZE);

  int numbytes;

  if((numbytes = recv(socket_fd,readbuf,MAXDATASIZE-1,0)) == -1){
    fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
    exit(1);
  }

  printf("numbytes =  %d\n", numbytes);
  readbuf[numbytes] = '\0';

  printf("connected client has pseudo : %s \n",readbuf);
  return readbuf;
  
}


blackjack_table* init_blackjack_table(int size){
  //printf("\ninside init_blackjack_table()\n");
  blackjack_table* pt = (blackjack_table*)malloc(sizeof(blackjack_table));
  pt->size = size;
  pt->number_of_players = 0; 
  pt->players = (player**)malloc(size * sizeof(player*));
  for(int i=0;i<size;i++){
    pt->players[i] = NULL;
  }
  pt->full = 0;
  return pt;
}

int add_player_to_table(blackjack_table* pt, player* p){
  //printf("\ninside add_player_to_table()\n");
  if(pt == NULL){
    printf("blackjack_table doesn't exist\n");
    return -1;
  }
  if(pt->full == 1){
    printf("blackjack table is full\n");
    return -1;
  }
  if(p == NULL){
    printf("player is null\n");
    return -1;
  }
  printf("number of players already at the table = %d\n",pt->number_of_players);
  pt->players[pt->number_of_players] = p;
  pt->number_of_players++;
  if(pt->number_of_players >= pt->size){
    printf("blakjack table became full\n");
    pt->full = 1;
  }
  return 1;
}

int remove_player_from_table(blackjack_table* pt, player* p, pseudo_db* pb){
  int found = 0;
  for(int i=0; i<pt->size; i++){
    if(p == pt->players[i]){
      //close the socket descriptor, unbind pseudo and free the player structure
      close(pt->players[i]->socket_fd);
      unbind_pseudo(&pb,pt->players[i]->pseudo);
      free(pt->players[i]);
      
      pt->players[i] = NULL;
      pt->number_of_players--;
      found = 1;
      for(int j= i; j < pt->size - 1; j++){
	pt->players[j] = pt->players[j+1];
      }
      pt->players[pt->size - 1] = NULL;
      if(pt->number_of_players < pt->size){
	pt->full = 0;
      }
      break;
    }
  }
  if(found == 1) return 1;
  else return -1;
}


int check_connectivity(player* p, int timeout){
  //printf("\ninside check_connectivity()\n");
  char* isconnected_msg = "req: connected";

  if(send(p->socket_fd,isconnected_msg,strlen(isconnected_msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return -1;
  }
  
  fd_set readfds;
  struct timeval t;
  t.tv_sec = timeout;
  t.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(p->socket_fd,&readfds);
  int rv = select(p->socket_fd + 1,&readfds,NULL,NULL,&t);
  if(rv == -1){
    fprintf(stderr,"select: error in select: %s\n",strerror(errno));
    return -1;
  }else if(rv == 0){
    printf("time out\n");
    return 0;
  }else{
    char recvbuf[5];
    int sz =  recv(p->socket_fd,recvbuf,5,0);
    if(sz == -1){
      fprintf(stderr,"recv: error in recv: %s\n",strerror(errno));
      return -1;
    }else if(sz == 0){
      printf("disconnected\n");
      return 0;
    }else{
      return 1;
    }
  }
}
    
