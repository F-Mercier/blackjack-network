#include "client_game.h"
#include <stdio.h>
#include <stdlib.h>



game_instance* init_game(int socket_fd, char* pseudo){
  game_instance* game= (game_instance*)malloc(sizeof(game_instance));
  game->socket_fd = socket_fd;
  game->my_pseudo = pseudo;
  int nr = fetch_max_players_no(socket_fd);
  game->number_of_players = nr;
  char** res = fetch_players_pseudos(socket_fd);
  game->all_players_pseudos = res;
  game->all_players_actions = (action*)malloc(nr*sizeof(action));
  for(int i=0; i<nr; i++){
    game->all_players_actions[i] = NULL;
  }
  
  
  
