#ifndef players_h
#define players_h

/*
 *structure defining a player
 * includes a socket descriptor and the pseudo plus future attributes
 */
struct player{
  int socket_fd; //socket descriptor associated with this client
  char* pseudo; // unique identifiant for the player along socket descriptor
  int connected; // boolean: 1 - player is connected ; 0 - not connected
};

typedef struct player player;

/*
 *structure containing all infos about a particular instance of the game
 * it includes all players at a "blackjack table" plus additional infos
 */
struct player_table{
  int curr_no_players; //current number of players at this table
  player* p; // array of players
  int full; //boolean: 1 - table is full ; 0 - table is not yet full
};

typedef struct players_table players_table;

/*
 *initialize a player structure
 *if pseudo is not good(already existent in the database) ask the client for another one
 *return a pointer to the newly created player
 */
player* init_player(int socket_fd, char* pseudo);

/*
 *this method will be called inside thread_manager add_player_table
 * argument size is passed by thread manager's max no of players at a table
 *return a pointer to a new created player_table
 */
player_table* init_players_table(int size);

/*
 *add a completly created player to a player_table
 *this method is called by thread_manager add_player method when adding a player as a mean to encapsulate structure logic
 */
void  add_player_to_table(players_table* pt, player* p);

/*
 *removes the selected player from the player_table becouse either client disconnected or game ended and the clent had chosen to close the app
 *the pseudo must be unbinded for future use
 */
void remove_player_from_table(players_table* pt, player* p);

/*
 *check if a client is connected
 *return 1 if this player is connected, else it returns 0
 */
int check_connectivity(player* p);


#endif //players_h
