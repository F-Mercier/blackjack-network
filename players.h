#ifndef players_h
#define players_h

#include "pseudos.h"


/*
 *structure defining a player
 * includes a socket descriptor and the pseudo plus future attributes
 */
typedef struct player{
  int socket_fd; //socket descriptor associated with this client
  char* pseudo; // unique identifiant for the player along socket descriptor
  int connected; // boolean: 1 - player is connected ; 0 - not connected
}player;

/*
 *structure containing all infos about a particular instance of the game
 * it includes all players at a "blackjack table" plus additional infos
 */
typedef struct players_table{
  int curr_no_players; //current number of players at this table
  int size; // size of the table (necessary for the loops)
  player** p; // array of players
  int full; //boolean: 1 - table is full ; 0 - table is not yet full
}players_table;

/*
 *initialize a player structure
 *if pseudo is not good(already existent in the database) ask the client for another one
 *return a pointer to the newly created player
 */
player* init_player(int socket_fd, pseudo_db* pb);

/*
 *this method will be called inside thread_manager add_player_table
 * argument size is passed by thread manager's max no of players at a table
 *return a pointer to a new created player_table
 */
players_table* init_players_table(int size);

/*
 *add a completly created player to a player_table
 *this method is called by thread_manager add_player method when adding a player as a mean to encapsulate structure logic
 * return -1 if pt is already full and a player cannont be added or 1 if it succeeds
 */
int add_player_to_table(players_table* pt, player* p);

/*
 *removes the selected player from the player_table becouse either client disconnected or game ended and the clent had chosen to close the app
 *the pseudo must be unbinded for future use
 *returns -1 if an error occured like there is no such player at that table
 *returns 1 on success
 */
int remove_player_from_table(players_table* pt, player* p);

/*
 *check if a client is connected
 *return 1 if this player is connected, returns 0 if disconected and -1 if check_connectivity fails
 */
int check_connectivity(player* p, int timeout);


/*
 *method to retrieve from the client another pseudo
 *uses socket_fd of the client to pass the messages
 *only used in the init_player method
 */
char* ask_for_pseudo(int socket_fd); 

#endif //players_h
