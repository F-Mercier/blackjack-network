#include <stdio.h>
#include <stdlib.h>

#include "threads_manager.h"

threads_manager* init_th_manager(int size, int no_players){
  //printf("\n inside init_th_manager()\n");
  threads_manager* tm = (threads_manager*)malloc(sizeof(threads_manager));
  tm->size = size;
  tm->index = 0; //before first element is inserted in the table
  tm->no_players = no_players;
  tm->tables = (blackjack_table**)malloc(size * sizeof(blackjack_table*));
  for(int i = 0; i<tm->size; i++){
    tm->tables[i] = NULL;
  }
  return tm;
}

void increase_size(threads_manager* tm){
  //printf("\n inside increase_size()\n");
  int old_size = tm->size;
  int new_size = old_size * 2;
  blackjack_table** tables =(blackjack_table**)malloc(new_size*sizeof(blackjack_table*));
  for(int i= 0; i<old_size; i++){
    tables[i] = tm->tables[i];
  }
  for(int j = old_size ;j<new_size;j++){
    tables[j] = NULL;
  }
  blackjack_table** tmp = tm->tables;
  tm->tables = tables;
  tm->size = new_size;
  free(tmp);
}

void add_blackjack_table(threads_manager* tm){
  //printf("\ninside add_blackjack_table()\n");
  if(tm->index >= tm->size - 1){
    increase_size(tm);
  }
  blackjack_table* pt = init_blackjack_table(tm->no_players);
  printf("blackjack_table initialized\n");
  tm->tables[tm->index] = pt;
  tm->index++;
}

int remove_blackjack_table(threads_manager* tm,int table_no){
  if(table_no < 0 || table_no > tm->index){
    fprintf(stderr,"invalid table_no argument\n");
    return -1;
  }
  free(tm->tables[table_no]);
  tm->tables[table_no] = NULL;
  for(int i = table_no;i<tm->index; i++){
    tm->tables[i] = tm->tables[i+1];
  }
  tm->tables[tm->index - 1] = NULL;
  tm->index--;
  return 1;
}

int add_player(threads_manager* tm, player* p){
  //printf("\ninside add_player()\n");
  if(tm->index == 0){//there is no allocated table
    printf("first table initialization\n");
    add_blackjack_table(tm);
  }
  if(add_player_to_table(tm->tables[tm->index - 1],p) == -1){
    add_blackjack_table(tm);
    add_player_to_table(tm->tables[tm->index - 1],p);
    return tm->index - 1;
  }
  return tm->index - 1;
}
  
int remove_player(threads_manager* tm,int table_no, player* p){
  if(remove_player_from_table(tm->tables[table_no],p) == -1) return -1;
  else return 1;
}


int check_clients_connectivity(threads_manager* tm, int table_no, int timeout){
  for(int i = 0; i<tm->tables[table_no]->number_of_players; i++){
    if(check_connectivity(tm->tables[table_no]->players[i],timeout) == 0){
      remove_player(tm,table_no,tm->tables[table_no]->players[i]);
    }else if(check_connectivity(tm->tables[table_no]->players[i],timeout) == -1){
      return -1;
    }
  }
  return 1;
}


void print_blackjack_tables(threads_manager* tm){
  //printf("\ninside print_blackjack_table\n");
  printf("number of blackjack tables = %d\n",tm->index); 
  for(int i = 0; i < tm->index ; i++){
    printf("Blackjack Table %d :\n{ ",i);
    //printf("size of the blackjack table = %d\n",tm->tables[i]->size);
    //printf("number of players already at the table = %d\n",tm->tables[i]->number_of_players);
    for(int j = 0; j < tm->tables[i]->size; j++){
      if(j < tm->tables[i]->number_of_players){
         printf(" %s:%d ",tm->tables[i]->players[j]->pseudo,tm->tables[i]->players[j]->socket_fd);
      }else{
         printf(" empty "); 
      }
    }
    printf("}\n\n");
  }
}
