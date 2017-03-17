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
  card_package_t* package = init_card_package();
  card_package_t* hpackage = init_card_package();//hidden cards
  for(int i=0; i<hpackage->counter; i++){
    hpackage->cards[i].hidden = 1;
  }

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
    if(m == first_card || m == second_card){
      printf("\n\nRECEIVE FIRST CARD.....OR SECOND CARD.....\n\n");
      char card[10];
      memset(card,0,10);
      int i;
      if(m == first_card) i=11;
      else i=12;
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
      	l++;
      }
      pseudonim[l] = '\0';
      
      //printf("before converting string to card\n");
      card_t fcard = string_to_card(card);
      //printf("test card: sym= %s, color= %s, val= %d, hidden= %d\n",fcard.symbol, fcard.color, fcard.value, fcard.hidden);
      //printf("before adding card to player %s hand\n",pseudonim);
      if(m==second_card && strcmp(pseudonim,"dealer")==0){
	for(int o = 0; o < 52; o++){
	  if(strcmp(fcard.symbol,hpackage->cards[o].symbol)==0 &&
	     strcmp(fcard.color,hpackage->cards[o].color)==0){
	    add_card_to_hand(game,&hpackage->cards[o],pseudonim);
	    break;
	  }
	}
      }else{
	for(int o = 0; o < 52; o++){
	  if(strcmp(fcard.symbol,package->cards[o].symbol)==0 &&
	     strcmp(fcard.color,package->cards[o].color)==0){
	    add_card_to_hand(game,&package->cards[o],pseudonim);
	    break;
	  }
	}
      }
      for(int p=0; p<game->number_of_players; p++){
	printf("%s, %s\n", card_to_string(game->players_cards[p][0]), card_to_string(game->players_cards[p][1]));
      }
    }

    if(m == req_bet){
      printf("enter the amount of money you want to bet : ");
      int b;
      scanf("%d",&b);
      int temp = b;
      int nr_digits=0;
      while(temp!=0){
	temp /= 10;
	nr_digits++;
      }
      char message[30];
      memset(message,0,30);
      sprintf(message,"send_bet:%d",b);
      printf(" bet message = %s",message);
      if(send(sockfd,message,strlen(message),0) == -1){
	fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	return;
      }

      for(int j = 0; j<game->number_of_players; j++){
	if(strcmp(game->players_pseudos[j],game->my_pseudo) == 0){
	  game->players_bets[j] = b;
	  break;
	}
      }
    }

    if(m == spread_bet){
      printf("registering the bet from another player\n");
      char pseudo[20];
      char bet[10];
      memset(bet,0,10);
      memset(pseudo,0,20);
      int i = 11;
      int k = 0;
      int l = 0;
      while(msg[i] != ';'){
	pseudo[k] = msg[i];
	k++;
	i++;
      }
      pseudo[k] = '\0';
      i++;
      while(msg[i] != ')'){
	bet[l] = msg[i];
	l++;
	i++;
      }
      bet[l] = '\0';
      int b = atoi(bet);
      printf("player %s has bet %d\n",pseudo,b);
      //registering the bet
      for(int j = 0; j<game->number_of_players; j++){
	if(strcmp(game->players_pseudos[j],pseudo) == 0){
	  game->players_bets[j] = b;
	  break;
	}
      }
    }

    if(m == play_turn){

      char act[6];
      memset(act,0,6);
      printf("What action do you choose? hit or stand ? : ");
      fgets(act,6,stdin);
      while(strncmp(act,"hit",3) != 0 && strncmp(act,"stand",5)!=0){
	printf("wrong choice! Try again hit or stand ? : ");
	memset(act,0,6);
	fgets(act,6,stdin);
	printf("%s\n",act);
      }

      if(send(sockfd,act,strlen(act),0) == -1){
	fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	return;
      }


    }

    if(m == update_stand){
      //do stuff to update client game
    }
    
    
    if(m == req_connected){
      send_keep_connection(sockfd);
    }

    printf("before printing the game\n\n");
    print_game(game);
    memset(msg,0,2*MAXDATASIZE);
    
  }

  return 0;
}
  
  

