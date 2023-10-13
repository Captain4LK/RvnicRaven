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
   RvR_array_length_set(turn_heap,0);

   //Push player first (to guarantee player is first to act)
   player.e->action_points = 128;
   turn_heap_push(player.e);

   Entity *cur = a->entities;
   for(;cur!=NULL;cur = cur->next)
   {
      cur->action_points = 128;
      entity_turn(w,a,cur);
      if(cur==player.e)
         continue;
      turn_heap_push(cur);
   }

   //TODO: should we run this every turn?

   //Delete removed entities
   Entity *e = a->entities;
   Entity *next = NULL;
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

void turns_do_until(World *w, Area *a, Entity *until)
{
   while(RvR_array_length(turn_heap)>0&&turn_heap[0]!=until)
   {
      Entity *e = turn_heap_max();
      if(e==NULL)
         return;

      //e->action.id = ACTION_INVALID;
      entity_think(w, a, e);
      if(e->action.id==ACTION_INVALID)
         continue;

      action_do(w,a,e);

      if(e->action_points>0)
         turn_heap_push(e);
   }
}

void turn_heap_push(Entity *e)
{
   RvR_array_push(turn_heap,e);
   int index = (int)RvR_array_length(turn_heap)-1;
   while(index>0&&turn_heap[(index-1)/2]->action_points<turn_heap[index]->action_points)
   {
      Entity *tmp = turn_heap[(index-1)/2];
      turn_heap[(index-1)/2] = turn_heap[index];
      turn_heap[index] = tmp;
      index = (index-1)/2;
   }
}

Entity *turn_heap_max(void)
{
   if(turn_heap==NULL||RvR_array_length(turn_heap)<1)
      return NULL;

   Entity *max = turn_heap[0];
   turn_heap[0] = turn_heap[RvR_array_length(turn_heap)-1];
   size_t len = RvR_array_length(turn_heap);
   RvR_array_length_set(turn_heap,len-1);
   if(RvR_array_length(turn_heap)==0)
      return NULL;

   int index = 0;
   for(;;)
   {
      int left = -1;
      int right = -1;
      if(2*index+1<RvR_array_length(turn_heap))
         left = turn_heap[2*index+1]->action_points;
      if(2*index+2<RvR_array_length(turn_heap))
         right = turn_heap[2*index+2]->action_points;

      if(turn_heap[index]->action_points>=left&&turn_heap[index]->action_points>=right)
         break;

      if(left>right)
      {
         Entity *tmp = turn_heap[2*index+1];
         turn_heap[2*index+1] = turn_heap[index];
         turn_heap[index] = tmp;
         index = 2*index+1;
      }
      else
      {
         Entity *tmp = turn_heap[2*index+2];
         turn_heap[2*index+2] = turn_heap[index];
         turn_heap[index] = tmp;
         index = 2*index+2;
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
