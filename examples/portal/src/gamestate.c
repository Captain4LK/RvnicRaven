/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "gamestate.h"
#include "entity.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Gamestate *gamestate_new()
{
   Gamestate *state = RvR_malloc(sizeof(*state),"GameState");
   memset(state,0,sizeof(*state));

   return state;
}

void gamestate_free(Gamestate *s)
{
   if(s==NULL)
      return;

   RvR_free(s);
}

void gamestate_map(Gamestate *s, uint16_t id)
{
   if(s==NULL||s->player.entity==NULL)
      return;

   s->map = RvR_port_map_load(id);
   s->cam.x = s->player.entity->x;
   s->cam.y = s->player.entity->y;
   s->cam.z = s->player.entity->z;
   s->cam.shear = 0;
   s->cam.fov = 2048;
   s->cam.sector = RvR_port_sector_update(s->map,0,s->cam.x,s->cam.y);
   entity_add(s,s->player.entity);
}
//-------------------------------------
