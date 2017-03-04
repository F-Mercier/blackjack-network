#include "players.h"
#include "pseudos.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#define MAXDATASIZE 100

player* init_player(int socket_fd, pseudo_db* pb){
  printf("inside init_player()\n");
  player* p = (player*)malloc(sizeof(player));
  p->socket_fd = socket_fd; //set the socket descriptor
  p->connected = 1; //set connected to true
  // check if pseudo already exists before we bind it to the player
  //steps : check_existance - bind_pseudo - set pseudo to player
  char* pseudo;
  pseudo = ask_for_pseudo(socket_fd);
  while(check_existance(pb,pseudo) == 0){
    pseudo = ask_for_pseudo(socket_fd);
  }
  bind_pseudo(&pb,pseudo);
  p->pseudo = pseudo;
  return p;
}


char* ask_for_pseudo(int socket_fd){
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


players_table* init_players_table(int size){
  players_table* pt = (players_table*)malloc(sizeof(players_table));
  pt->size = size;
  pt->curr_no_players = 0; // it is like index = -1 in the array because no elements
  pt->p = (player**)malloc(size * sizeof(player*));
  for(int i=0;i<size;i++){
    pt->p[i] = NULL;
  }
  pt->full = 0;
  return pt;
}

int add_player_to_table(players_table* pt, player* p){
  if(pt->full == 1) return -1;
  pt->p[pt->curr_no_players] = p;
  pt->curr_no_players++;
  if(pt->curr_no_players == pt->size) pt->full = 1;
  return 1;
}

int remove_player_from_table(players_table* pt, player* p){
  int found = 0;
  for(int i=0; i<pt->size; i++){
    if(p == pt->p[i]){
      free(pt->p[i]);
      pt->p[i] = NULL;
      pt->curr_no_players--;
      found = 1;
      for(int j= i; j < pt->size - 1; j++){//FIXME check with curr_no_players
	pt->p[j] = pt->p[j+1];
      }
      pt->p[pt->size - 1] = NULL;
      break;
    }
  }
  if(found == 1) return 1;
  else return -1;
}

//FIXME check if player is NULL
int check_connectivity(player* p, int timeout){
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
    
