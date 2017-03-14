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
  char pseudo[20];
  memset(pseudo,0,20);
  printf("before first ask for pseudo\n");
  ask_for_pseudo(socket_fd,pseudo);
  printf("pseudo is %s\n",pseudo);
  if(check_existance(pb,pseudo) == 0){
    printf("checking existance for %s\n",pseudo);
    bind_pseudo(&pb,pseudo);
    send_pseudo_confirmation(socket_fd);
  }else{
    printf("check_existance for %s returned 1\n",pseudo);
    while(check_existance(pb,pseudo) != 0){
      printf("checking existance for %s in loop\n",pseudo);
      memset(pseudo,0,20);
      ask_for_pseudo(socket_fd,pseudo);
    }
    bind_pseudo(&pb,pseudo);
    send_pseudo_confirmation(socket_fd);
  }
  printf("now pseudo is %s\n",pseudo);
  strncpy(p->pseudo,pseudo,20);
  p->card1 = NULL;
  p->card2 = NULL;
  p->money = 500;
  p->bet = 0;
  p->act = NO_ACTION;
  p->card_sum = 0;
  
  return p;
}


char* ask_for_pseudo(int socket_fd, char* pseudo){
  //printf("\ninside ask_pseudo()\n ");

   char* init_msg = "10:req_pseudo";

   if(send(socket_fd,init_msg,strlen(init_msg),0) < 0){
     fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
     exit(1); 
   }
  
   char readbuf[MAXDATASIZE];
   memset(readbuf,0,MAXDATASIZE); 
   int numbytes;
   int bytes;
   int num_buf[4];
   memset(num_buf,0,4);
   if((bytes = recv(socket_fd,num_buf,3,0)) == -1){
     fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
     exit(1);
   }
   num_buf[bytes-1] = '\0';
   int sz = atoi(num_buf);
   printf("we read %d for pseudo\n",sz);
   
   printf("receving pseudo from client\n");
   if((numbytes = recv(socket_fd,readbuf,sz,0)) < 0){
     fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
     exit(1);
    
   }
   printf("numbytes =  %d\n", numbytes);
   readbuf[numbytes] = '\0';
   
   printf("connected client has pseudo : %s \n",readbuf);
   strncpy(pseudo,readbuf,sz);
   return readbuf;
   
}

void send_pseudo_confirmation(int socket_fd){
  char* init_msg = "14:pseudo_enabled";
  if(send(socket_fd,init_msg,strlen(init_msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return;
  }
}


void send_players_info(blackjack_table* table, int socket_fd){
  printf("inside send_player_info() \n");

  if(table->number_of_players == 0){
    printf("no player at the table\n");
    return;
  }
  
  int n = table->size * 20 + table->size * 3;
  char message[n];
  //message ex:
  //1:pseudo1;;2:pseudo2;;
  memset(message,0,n);
  char str[40];
  memset(str,0,40);
  
  sprintf(str,"players_info=%d:%s;;",0,table->players[0]->pseudo);
  //printf("aaa\n");
  strncpy(message,str,40);
  //printf("temp = %s\n",message);
  for(int i=1; i< table->number_of_players; i++){
    memset(str,0,40);
    sprintf(str,"%d:%s;;",i,table->players[i]->pseudo);
    //printf("ccc\n");
    strncat(message,str,40);
    //printf("ddd\n");
  }
  printf("message = %s\n",message);
  int m = n+3;
  char final_message[m];
  memset(final_message,0,m);
  char length[2];
  memset(length,0,2);
  if(length < 10){
    sprintf(length,"0%d:",strlen(message));
  } else{
    sprintf(length,"%d:",strlen(message));
  }
  strncpy(final_message,length, strlen(length));
  strncat(final_message, message, strlen(message));
  printf("final message = %s\n",final_message);

  if(send(socket_fd,final_message,strlen(final_message),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return;
  }
  
}

void send_start_game(int socket_fd){
  printf("inside send_start_game()\n");
  char* start_game = "10:start_game";
  if(send(socket_fd,start_game,strlen(start_game),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return;
  }
  
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
  pt->count_views = 0;
  pt->info_changed = NO_INFO;
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


