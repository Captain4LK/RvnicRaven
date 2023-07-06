/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "player.h"
#include "world_defs.h"
#include "entity.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Player player;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_new(World *w, Area *a)
{
   player.e = entity_new(w);
   player.e->x = 0;
   player.e->y = 0;
   player.e->z = 15;
   player.e->speed = 32;
   entity_add(a,player.e);
}

void player_update()
{
}
//-------------------------------------
