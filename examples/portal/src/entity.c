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
static Entity *entity_pool = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Entity *entity_new()
{
   if(entity_pool==NULL)
   {
      Entity *ne = RvR_malloc(sizeof(*ne) * 256, "Entity pool");
      memset(ne, 0, sizeof(*ne) * 256);

      for(int i = 0; i<256 - 1; i++)
         ne[i].next = &ne[i + 1];
      entity_pool = &ne[0];
   }

   Entity *n = entity_pool;
   entity_pool = n->next;
   n->next = NULL;
   n->prev_next = NULL;

   uint64_t gen = n->generation;
   memset(n, 0, sizeof(*n));
   n->generation = gen;

   return n;
}

void entity_free(Entity *e)
{
   if(e==NULL)
      return;

   //ai_free(e);
   //if(e->cards!=NULL)
      //RvR_free(e->cards);
   //grid_entity_remove(e);

   *e->prev_next = e->next;
   if(e->next!=NULL)
      e->next->prev_next = e->prev_next;

   e->next = entity_pool;
   entity_pool = e;
}

void entity_add(Gamestate *state, Entity *e)
{
   if(e==NULL)
      return;

   e->prev_next = &state->entities;
   if(state->entities!=NULL)
      state->entities->prev_next = &e->next;
   e->next = state->entities;
   state->entities = e;
}

void entity_remove(Entity *e)
{
   if(e==NULL)
      return;

   e->generation++;
   e->removed = 1;
}
//-------------------------------------
