#ifndef threads_manager
#define threads_manager


#include "players.h"

/**structure that contains infos about the clients present at each table (socket,pseudo, etc )
 *it is an array indexed by the number/id of the "blackjack table"
 */
struct threads_manager{
  int size; //total number of "blackjack tables"
  players_table* tables; //an array containing all "blakjack tables" (games)
  int index; //used to keep track of the actual number of elements in tables array
  int table_no; //helps giving a player_table a unique number/id - it is only incremented
  int no_players; // max number of players at a particular "blackjack table"
};

typedef struct threads_manager threads_manager;

/**
 *initialize a thread_manager structure
 *param[in] size = initial size of the array
 *param[in] no_player = max number of players at a "blackjack table"
 *return a pointer to the new created thread_manager
 *table_no initialized at 1
 *index initialized at 0
 */
threads_manager* init_th_manager(int size, int no_players);
  
/**
 *increases the number of "blackjack tables" when necessary
 */
void increse_size(threads_manager* tm);

/**
 *adds a new "blackjack table" in tables array
 *if the current array is full it is increased by increase_size method before adding the new "blackjack table" 
 */
void add_players_table(threads_manager* tm);

/*
 *removes the content of te selected player_table along with all the players
 */
void remove_player_table(threads_manager* tm,int table_no);

/**
 *add a new player(thread) to the thread manager players_table array
 *player already created, and his pseudo is already binded
 *if all players_table are full create a new players_table and add the client there  
 */
void add_player(threads_manager* tm, player* p);

/**
 * removes a player from the players_table array
 * calls the remove_player_from_table method
 * is called by check_clients_connectivity when a client is disconnected
 */
void remove_player(threads_manager* tm, player* p);

/**
 *scan all clients on selected players_table for connectivity
 *if one is disconnected it is removed from the table
 */
void check_clients_connectivity(threads_manager* tm, players_table* pt);


/**
 *prints the content of threads_manager's tables array
 */
void print_players_table_array(threads_manager* tm);

#endif // threads_manager