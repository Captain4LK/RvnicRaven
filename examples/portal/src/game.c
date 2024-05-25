/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "state.h"
#include "game.h"
#include "gamestate.h"
#include "player.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Gamestate *gamestate = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_set()
{
   gamestate_free(gamestate);
   gamestate = gamestate_new();
   player_create_new(gamestate);
   gamestate_map(gamestate,0);
}

void game_init()
{
}

void game_update()
{
   player_update(gamestate);
}

void game_draw()
{
   RvR_port_draw_begin(gamestate->map,&gamestate->cam);

      RvR_port_draw_map(NULL);

   RvR_port_draw_end(NULL);
}
//-------------------------------------
