/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "state.h"
#include "title.h"
#include "game.h"
#include "game_card.h"
#include "game_inventory.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static State state;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void state_init(State s)
{
   switch(s)
   {
   case STATE_TITLE: state = s; title_init(); break;

   case STATE_GAME: state = s; game_init(); break;
   case STATE_GAME_CARD: state = s; game_card_init(); break;
   case STATE_GAME_CARD_VIEW: state = s; game_card_view_init(); break;
   case STATE_GAME_CARD_INVENTORY: state = s; game_card_inventory_init(); break;

   case STATE_GAME_INVENTORY: state = s; game_inventory_init(); break;
   }

   state_set(s);
}

void state_set(State s)
{
   switch(s)
   {
   case STATE_TITLE: state = s; title_set(); break;

   case STATE_GAME: state = s; game_set(); break;
   case STATE_GAME_CARD: state = s; game_card_set(); break;
   case STATE_GAME_CARD_VIEW: state = s; game_card_view_set(); break;
   case STATE_GAME_CARD_INVENTORY: state = s; game_card_inventory_set(); break;

   case STATE_GAME_INVENTORY: state = s; game_inventory_set(); break;
   }
}

void state_draw()
{
   switch(state)
   {
   case STATE_TITLE: title_draw(); break;

   case STATE_GAME: game_draw(); break;
   case STATE_GAME_CARD: game_card_draw(); break;
   case STATE_GAME_CARD_VIEW: game_card_view_draw(); break;
   case STATE_GAME_CARD_INVENTORY: game_card_inventory_draw(); break;

   case STATE_GAME_INVENTORY: game_inventory_draw(); break;
   }
}

void state_update()
{
   switch(state)
   {
   case STATE_TITLE: title_update(); break;

   case STATE_GAME: game_update(); break;
   case STATE_GAME_CARD: game_card_update(); break;
   case STATE_GAME_CARD_VIEW: game_card_view_update(); break;
   case STATE_GAME_CARD_INVENTORY: game_card_inventory_update(); break;

   case STATE_GAME_INVENTORY: game_inventory_update(); break;
   }
}
//-------------------------------------
