#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "card.h"

typedef struct game_instance{
  int socket_fd;//socket descriptor to speak with the server
  char* my_pseudo;
  char** all_players_pseudos;
  action* all_player_actions;//this vector is updated at each tour
  int my_tour_number;
  card_t cards_in_hand[20];//all cards in the hand starting with the first two given by the dealer
  int cards_sum;
  int waiting;//if we have to wait in order that the game begin
  //int number_of_players;
}game_instance;

//this method initialize the game instance with some default values then
//fetches all the other values drom the server
game_instance* init_game(int socket_fd, char* pseudo);

//defines the main loop of the game executed in the client
//all calls to the server are done through this loop
void main_loop(game_instance* gi);

message receive_server_message(game_instance* gi);

//send a hit action to the server
void hit(game_instance* gi);

//send a stand action to the server
void stand(game_instance* gi);

//update the content of the data structure before printing
void update(game_instance* gi);

//fetch the players name from the server
char** fetch_players_pseudos(int socket_fd);

//get the maximum number of players at a table
int fetch_max_players_no(int socket_fd);

//get if the table is full so that the game can start
int is_table_full(int socket_fd);

//sends a chosen pseudo over the network
void send_pseudo(int socket_fd, char* pseudo);

//manage the graphical output for the game
void print_game(game_instance* gi);

#endif//CLIENT_GAME_H
