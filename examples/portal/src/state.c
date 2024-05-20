/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "state.h"
#include "game.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static State state = STATE_INVALID;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void state_set(State s)
{
   switch(s)
   {
   case STATE_INVALID: state = s; break;
   case STATE_MENU: state = s; break;
   case STATE_GAME: state = s; game_set(); break;
   }
}

void state_init(State s)
{
   switch(s)
   {
   case STATE_INVALID: state = s; break;
   case STATE_MENU: state = s; break;
   case STATE_GAME: state = s; game_init(); break;
   }
}

void state_update()
{
   switch(state)
   {
   case STATE_INVALID: break;
   case STATE_MENU: break;
   case STATE_GAME: game_update(); break;
   }
}

void state_draw()
{
   switch(state)
   {
   case STATE_INVALID: break;
   case STATE_MENU: break;
   case STATE_GAME: game_draw(); break;
   }
}
//-------------------------------------
