#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


card_t init_card(char symbol[], char color, int value, int hidden){
  card_t card;
  strncpy(card.symbol,symbol,3);
  card.color = color;
  card.value = value;
  card.hidden = hidden;
  return card;
}

card_package_t* init_card_package(){
  card_package_t* package = (card_package_t*)malloc(sizeof(card_package_t));
  char symbols[52][3]={"A","A","A","A","2","2","2","2","3","3","3","3","4","4","4","4","5","5","5","5","6","6","6","6","7","7","7","7","8","8","8","8","9","9","9","9","10","10","10","10","J","J","J","J","Q","Q","Q","Q","K","K","K","K"};
  char colors[52]={'H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D','H','S','C','D'};
  int values[52] ={1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
  for(int i = 0; i<52; i++){
    package->cards[i] = init_card(symbols[i],colors[i],values[i],0);
  }
  package->counter = 52;//init counter with the number of cards
  return package;
}

void shuffle_cards(card_package_t* cp){
  for(int i = 0; i < 51; i++){
    int j = rand()%(i+1);
    if(j != i){
      card_t tmp = cp->cards[i];
      cp->cards[i] = cp->cards[j];
      cp->cards[j] = tmp;
    }
  }
}

void reveal_card(card_t* card){
  card->hidden = 0;
}

void hide_card(card_t* card){
  card->hidden = 1;
}

card_t* get_card(card_package_t* cp){
  if(cp->counter > 0){
    card_t* c = &cp->cards[cp->counter - 1];
    cp->counter--;
    return c;
  }else{
    return NULL;
  }
}

void print_card_package(card_package_t* cp){
  for(int i = 0; i<cp->counter; i++){
    printf("%s %c | ",cp->cards[i].symbol,cp->cards[i].color);
  }
  printf("\n");
}
