/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "config.h"
#include "entity.h"
#include "item.h"
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
Entity **turn_heap = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void turn_start(World *w, Area *a)
{
   //TODO: should we run this every turn?

   //Delete removed entities
   /*Entity *e = a->entities;
   Entity *next = NULL;
   for(; e!=NULL; e = next)
   {
      next = e->next;

      if(e->removed)
         entity_free(e);
   }*/

   //Delete removed items
   /*Item *it = a->items;
   Item *inext  = NULL;
   for(;it!=NULL;it = inext)
   {
      inext = it->next;

      if(it->removed)
         item_free(it);
   }*/

   RvR_array_length_set(turn_heap, 0);

   //Push player first (to guarantee player is first to act)
   player.e->action_points += 128;
   if(player.e->action_points>0)
      turn_heap_push(player.e);

   for(int c = 0;c<AREA_DIM*AREA_DIM*AREA_DIM;c++)
   {
      for(Entity *next = NULL, *cur = a->chunks[c]->entities; cur!=NULL; cur = next)
      {
         next = cur->next;

         Entity_index ind = entity_index_get(cur);
         entity_turn(w, a, cur);
         if(entity_index_try(ind)==NULL||cur==player.e)
            continue;

         cur->action_points += 128;
         if(cur->action_points<=0)
            continue;

         turn_heap_push(cur);
      }
   }
}

void turns_do_until(World *w, Area *a, Entity *until)
{
   while(RvR_array_length(turn_heap)>0&&turn_heap[0]!=until)
   {
      Entity *e = turn_heap_max();
      Entity_index ind = entity_index_get(e);
      if(e==NULL)
         return;

      entity_think(w, a, e);
      if(e->action.id==ACTION_INVALID)
      {
         e->action_points = 0;
         continue;
      }

      action_do(w, a, e);

      if(entity_index_try(ind)!=NULL&&e->action_points>0)
         turn_heap_push(e);
   }
}

void turn_heap_push(Entity *e)
{
   RvR_array_push(turn_heap, e);
   size_t index = RvR_array_length(turn_heap) - 1;
   size_t parent = (index - 1) / 2;
   while(index!=0&&turn_heap[parent]->action_points<turn_heap[index]->action_points)
   {
      Entity *tmp = turn_heap[parent];
      turn_heap[parent] = turn_heap[index];
      turn_heap[index] = tmp;
      index = parent;
      parent = (index - 1) / 2;
   }
}

Entity *turn_heap_max(void)
{
   if(turn_heap==NULL||RvR_array_length(turn_heap)<1)
      return NULL;

   Entity *max = turn_heap[0];
   turn_heap[0] = turn_heap[RvR_array_length(turn_heap) - 1];
   size_t len = RvR_array_length(turn_heap);
   RvR_array_length_set(turn_heap, len - 1);
   if(RvR_array_length(turn_heap)==0)
      return max;

   int index = 0;
   for(;;)
   {
      int left = -1;
      int right = -1;
      if(2 * index + 1<RvR_array_length(turn_heap))
         left = turn_heap[2 * index + 1]->action_points;
      if(2 * index + 2<RvR_array_length(turn_heap))
         right = turn_heap[2 * index + 2]->action_points;

      if(turn_heap[index]->action_points>=left&&turn_heap[index]->action_points>=right)
         break;

      if(left>right)
      {
         Entity *tmp = turn_heap[2 * index + 1];
         turn_heap[2 * index + 1] = turn_heap[index];
         turn_heap[index] = tmp;
         index = 2 * index + 1;
      }
      else
      {
         Entity *tmp = turn_heap[2 * index + 2];
         turn_heap[2 * index + 2] = turn_heap[index];
         turn_heap[index] = tmp;
         index = 2 * index + 2;
      }
   }

   return max;
}

Entity *turn_heap_peek_max(void)
{
   if(turn_heap==NULL||RvR_array_length(turn_heap)==0)
      return NULL;
   return turn_heap[0];
}
//-------------------------------------
