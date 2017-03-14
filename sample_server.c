#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "threads_manager.h"

#define PORT "5001"
#define BACKLOG 20
#define MAXDATASIZE 100

pthread_mutex_t mutex;
threads_manager tm;
pseudo_db pb;

// structure used to pass as argument in thread function
struct data_s{
  int* socket_fd;
  threads_manager* tm;
  pseudo_db* pb;
};
typedef struct data_s data;


void* run_game(void* arg){
  blackjack_table* table = (blackjack_table*)arg;
  initialize_game(table);
  while(1){
    //printf("inside main game loop\n");
  }
}


void* manage_games(void* arg){
  pthread_t game_thread;
  pthread_attr_t att;
  pthread_attr_init(&att);
  int i=0;
  int rc;
  while(1){
    if(tm.tables[i] != NULL){
      i++;
      //create new thread with the instance of the game
      rc = pthread_create(&game_thread,&att, run_game ,&(tm.tables[i]));
      if (rc){
	printf("ERROR; return code from pthread_create() is %d\n", rc);
	exit(-1);
      }
      printf("game instance created\n");
    }
  }
}


// This function is executed in every thread and should contain the logic of the player
void* run_thread(void* args){

  //extract useful data from the args argument 
  /* data* d = (data*)args; */
  /* int socket_fd = *(d->socket_fd); */
  /* threads_manager* tm =  d->tm; */
  /* pseudo_db* pb = d->pb; */
  
  int socket_fd = *(int*)args;
  
  //create a new player containig the socket descriptor and his pseudo
  player* p = init_player(socket_fd,&pb);
  pthread_mutex_lock(&mutex);
  //add player to a blackjack table
  int table_no = add_player(&tm,p);
  pthread_mutex_unlock(&mutex);
  print_pseudos(&pb);
  
  //print_blackjack_tables(tm);
  int is_running = 0;
  
  while(1){
    //check if full is 1 to tell client game started
    printf("inside while loop\n");
    if(tm.tables[table_no]->full == 1 && is_running == 0 ){
      printf("sending player infos\n");
      send_players_info(tm.tables[table_no],socket_fd);
      
      printf("sending start game signal\n");
      send_start_game(socket_fd);// to implement
      is_running = 1;
    }

    
    pthread_mutex_lock(&mutex);
    if(check_connectivity(p,15) == 0){
      send_disconnected_to_all(tm.tables[table_no],p);//to implement
      remove_player(&tm,table_no,p,&pb);
    }
    pthread_mutex_unlock(&mutex);
    print_blackjack_tables(&tm);
    sleep(1);//wait 1 second between frames
  }
  
  pthread_exit(NULL);
}


int main(int argc, char** argv){
  int sockfd, new_sockfd;
  struct addrinfo hints, *serverinfo, *p;
  struct sockaddr_storage client_addrs;
  socklen_t sin_size;
  int yes = 1;
  char ip[INET6_ADDRSTRLEN];
  int status;

  pthread_t thread, thread1;
  pthread_attr_t att;
  pthread_attr_init(&att);
  pthread_mutex_init(&mutex, NULL);

  if(argc != 2){
    fprintf(stderr, "%s: usage: sample_server max_player_on_table\n", argv[0]);
    return 1;
  }

  threads_manager* th_man = init_th_manager(100,atoi(argv[1]));
  pseudo_db* pseudos = init_pseudo_db(100);
  tm = *th_man;
  pb = *pseudos;

  //data thread_data;
  
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

  //before we accept we create a new thread to instantiate games
  //every time a new blackjack table is created this thread creates
  //a new thread that speaks through blackjack_table structure to
  //the client
  rc = pthread_create(&thread1,&att, manage_games ,NULL);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }
  
 
  while(1){
    sin_size = sizeof(client_addrs);
    new_sockfd = accept(sockfd,(struct sockaddr*)&client_addrs, &sin_size);
    if(new_sockfd < 0){
      fprintf(stderr, "accept: error while accepting: %s\n",strerror(errno));
      continue;
    }

    

    printf("accepted\n");

    //initialize the data we pass into the function executed in each thread
    /* thread_data.socket_fd = &new_sockfd; */
    /* thread_data.tm = tm; */
    /* thread_data.pb = pb; */

    rc = pthread_create(&thread,&att, run_thread , &new_sockfd);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
    
    t++;

  }

  //close thread and mutex resources
  pthread_mutex_destroy(&mutex);
  pthread_exit(NULL);
  return 0;
}
  
