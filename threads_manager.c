#include <stdio.h>
#include <stdlib.h>

#include "threads_manager.h"

threads_manager* init_th_manager(int size, int no_players){
  threads_manager* tm = (threads_manager*)malloc(sizeof(threads_manager));
  tm->size = size;
  tm->index = -1; //before first element is inserted in the table
  tm->no_players = no_players;
  tm->tables = (players_table**)malloc(size * sizeof(players_table*));
  for(int i = 0; i<tm->size; i++){
    tm->tables[i] = NULL;
  }
  return tm;
}

void increase_size(threads_manager* tm){
  int old_size = tm->size;
  int new_size = old_size * 2;
  players_table** tables =(players_table**)malloc(new_size*sizeof(players_table*));
  for(int i= 0; i<old_size; i++){
    tables[i] = tm->tables[i];
  }
  for(int j = old_size ;j<new_size;j++){
    tables[j] = NULL;
  }
  players_table** tmp = tm->tables;
  tm->tables = tables;
  tm->size = new_size;
  free(tmp);
}

void add_players_table(threads_manager* tm){
  if(tm->index >= tm->size - 1){
    increase_size(tm);
  }
  players_table* pt = init_players_table(tm->no_players);
  tm->tables[tm->index + 1] = pt;
  tm->index++;
}

int remove_player_table(threads_manager* tm,int table_no){
  if(table_no < 0 || table_no > tm->index){
    fprintf(stderr,"invalid table_no argument\n");
    return -1;
  }
  free(tm->tables[table_no]);
  tm->tables[table_no] = NULL;
  for(int i = table_no;i<tm->index; i++){
    tm->tables[i] = tm->tables[i+1];
  }
  tm->tables[tm->index] = NULL;
  tm->index--;
  return 1;
}

void add_player(threads_manager* tm, player* p){
  if(add_player_to_table(tm->tables[tm->index],p) == -1){
    add_players_table(tm);
    add_player_to_table(tm->tables[tm->index],p);
  }
}

int remove_player(threads_manager* tm,int table_no, player* p){
  if(remove_player_from_table(tm->tables[table_no],p) == -1) return -1;
  else return 1;
}


int check_clients_connectivity(threads_manager* tm, int table_no, int timeout){
  for(int i = 0; i<tm->tables[table_no]->curr_no_players; i++){
    if(check_connectivity(tm->tables[table_no]->p[i],timeout) == 0){
      remove_player(tm,table_no,tm->tables[table_no]->p[i]);
    }else if(check_connectivity(tm->tables[table_no]->p[i],timeout) == -1){
      return -1;
    }
  }
  return 1;
}


void print_players_table_array(threads_manager* tm){
  for(int i = 0; i < tm->index ; i++){
    printf("{ ");
    for(int j = 0; j < tm->tables[i]->size; j++){
      printf(" %s:%d ",tm->tables[i]->p[j]->pseudo,tm->tables[i]->p[j]->socket_fd);
    }
    printf("}\n");
  }
}
