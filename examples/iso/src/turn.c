/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "entity.h"
#include "world_defs.h"
#include "action.h"
#include "player.h"
#include "turn.h"
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

void turn_do(World *w, Area *a)
{
   Entity *e = a->entities;
   Entity *next = NULL;
   for(; e!=NULL; e = next)
   {
      next = e->next;

      if(e->removed)
         continue;
      if(e==player.e)
         continue;

      e->action_points = e->speed;
      while(e->action_points>0)
      {
         //entity_turn(e)

         if(e->action.id==ACTION_INVALID)
            break;

         action_do(w,a, e);
      }
   }

   //TODO: should we run this every turn?

   //Delete removed entities
   e = a->entities;
   next = NULL;
   for(; e!=NULL; e = next)
   {
      next = e->next;

      if(e->removed)
         entity_free(e);

      e = next;
   }

   /*//Delete removed items
   Item_world *it = items;
   while(it != NULL)
   {
      Item_world *next = it->next;

      if(it->removed)
         item_world_free(it);

      it = next;
   }*/
}
//-------------------------------------
