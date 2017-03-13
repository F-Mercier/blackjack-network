#include "client_game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXDATASIZE 100

int global = 0;

//this method should wait for all players to join the game before
//ending and entering the main loop 
game_instance* init_game(int socket_fd, char* pseudo){
  game_instance* game = (game_instance*)malloc(sizeof(game_instance));
  game->socket_fd = socket_fd;
  game->my_pseudo = pseudo;

  //setting up the client's pseudo
  char msg[MAXDATASIZE];
  memset(msg,0,MAXDATASIZE);
  message m = get_message(socket_fd,msg,MAXDATASIZE);
  while(m != pseudo_enabled){
    printf("inside pseudo init while loop\n");
    printf("message received: %s\n",msg);
    if(m == req_pseudo){
      printf("sending pseudo to server\n");
      send_pseudo(socket_fd, pseudo);
    }else if(m == req_other_pseudo){
      printf("retrying to send pseudo to server\n");
      send_other_pseudo(socket_fd,pseudo);
      game->my_pseudo = pseudo;
    }else{
      printf("message not recognized\n");
    }
    memset(msg,0,MAXDATASIZE);
    m = get_message(socket_fd,msg,MAXDATASIZE);
  }
  //ask the server how many players will be at the table
  //game->number_of_players = fetch_number_of_players(int socket_fd);
  printf("pseudo resolved\n");
  while( m != start_game){
    //wait for players to join the game
    printf("inside waiting loop...\n");
    memset(msg,0,MAXDATASIZE);
    m = get_message(socket_fd,msg,MAXDATASIZE);
    printf("received from server '%s'\n",msg);
    if(m == game_info){
      //read the game info message
      printf("game info message:\n %s\n",msg);
      if(send(socket_fd,"ack",3,0) < 0){
	fprintf(stderr,"send error:%s\n",strerror(errno));
	exit(1);
      }
    }else if(m == req_connected){
      printf("requested connection validation\n");
      send_keep_connection(socket_fd);
    }
  }


}

message get_message(int socket_fd,char* message, int size){
  char rbuf[MAXDATASIZE];
  int numbytes;
  int bytes;
  int size_d;
  char size_data[4];//maximum 99 caracters can be read need for : and \0 
  
  memset(rbuf,0,MAXDATASIZE);
  memset(size_data,0,4);
  printf("before reading from the server\n");
  //read the header on 2 chars
  //read something like 4:toto or 11:req: pseudo
  if((bytes = recv(socket_fd,size_data,3,0)) == -1){
    fprintf(stderr,"recv : error while reading from the server : %s\n",strerror(errno));
    exit(1);
  }
  size_data[bytes-1] = '\0';
  size_d = atoi(size_data);
  printf("We should read : %d bytes of data\n",size_d);
  
  if((numbytes = recv(socket_fd,rbuf,size_d,0)) == -1){
    fprintf(stderr,"recv : error while reading from the server : %s\n",strerror(errno));
    exit(1);
  }
  rbuf[numbytes] = '\0';
  printf("get_message(): received '%s'\n",rbuf);
  strncpy(message,rbuf,size_d);

  if(strncmp(rbuf,"req_pseudo",10) == 0 && global == 0){
    global++;
    printf("req_pseudo\n");
    return req_pseudo;
  }else if (strncmp(rbuf,"req_pseudo",10) == 0 && global != 0){
    printf("req: pseudo -- other\n");
    global++;
    return req_other_pseudo;
  }else if(strncmp(rbuf,"req_connected",13) == 0){
    printf("req_connected\n");
    return req_connected;
  }else if(strncmp(rbuf,"pseudo_enabled",14) == 0){
    printf("pseudo_enabled\n");
    return pseudo_enabled;
  }else if(strncmp(rbuf,"start_game",10) == 0){
    printf("start_game\n");
    return start_game;
  }else if(strncmp(rbuf,"game_info--",11)==0){
    //printf("game info message:\n");
    return game_info;
  }else return unknown;
  
}

void send_pseudo(int socket_fd,char* pseudo){
  int plen = strlen(pseudo);
  int n = plen+3;
  char msg[n];
  if(plen < 10){
    sprintf(msg,"0%d:%s",plen,pseudo);
  }else{
    sprintf(msg,"%d:%s",plen,pseudo);
  }
  if(send(socket_fd,msg,strlen(msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
  printf("finish sending\n");
  
}

void send_other_pseudo(int socket_fd, char* pseudo){
  printf("The pseudo %s is already taken. Please enter a new one : ",pseudo);
  fgets(pseudo,19,stdin);//one byte for '\0' terminator
  if ((strlen(pseudo)>0) && (pseudo[strlen (pseudo) - 1] == '\n'))
    pseudo[strlen (pseudo) - 1] = '\0';
  printf("you entered : %s\n",pseudo);
  while(strcmp(pseudo,"") == 0){
    printf("empty pseudo not accepted. Please enter again:");
    fgets(pseudo,19,stdin);
    if ((strlen(pseudo)>0) && (pseudo[strlen (pseudo) - 1] == '\n'))
      pseudo[strlen (pseudo) - 1] = '\0';
    printf("you entered : %s\n",pseudo);
  }
  int plen = strlen(pseudo);
  int n = plen+3;
  char msg[n];
  if(plen < 10){
    sprintf(msg,"0%d:%s",plen,pseudo);
  }else{
    sprintf(msg,"%d:%s",plen,pseudo);
  }
  
  if(send(socket_fd,msg,strlen(msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
  printf("finish sending\n");
}

void send_keep_connection(int socket_fd){
  if(send(socket_fd,"3:yes",5,0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
}


