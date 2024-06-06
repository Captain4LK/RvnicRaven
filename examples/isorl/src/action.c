/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "action.h"
#include "area.h"
#include "entity.h"
#include "astar.h"
#include "item.h"
#include "entity_documented.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static int action_move(World *w, Area *a, Entity *e);
static int action_wait(World *w, Area *a, Entity *e);
static int action_ascend(World *w, Area *a, Entity *e);
static int action_descend(World *w, Area *a, Entity *e);
static int action_attack(World *w, Area *a, Entity *e);
static int action_path(World *w, Area *a, Entity *e);
static int action_pickup(World *w, Area *a, Entity *e);
static int action_drop(World *w, Area *a, Entity *e);
static int action_equip(World *w, Area *a, Entity *e);
static int action_remove(World *w, Area *a, Entity *e);
static int action_put(World *w, Area *a, Entity *e);
//-------------------------------------

//Function implementations

int action_do(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   e->action_points -= act->cost;

   Action_id act_id = act->id;
   act->id = ACTION_INVALID;

   int status = 0;
   switch(act_id)
   {
   case ACTION_WAIT:
      status = action_wait(w, a, e);
      break;
   case ACTION_MOVE:
      status = action_move(w, a, e);
      break;
   case ACTION_ASCEND:
      status = action_ascend(w, a, e);
      break;
   case ACTION_DESCEND:
      status = action_descend(w, a, e);
      break;
   case ACTION_ATTACK:
      status = action_attack(w, a, e);
      break;
   case ACTION_PATH:
      status = action_path(w, a, e);
      break;
   case ACTION_PICKUP:
      status = action_pickup(w, a, e);
      break;
   case ACTION_DROP:
      status = action_drop(w, a, e);
      break;
   case ACTION_EQUIP:
      status = action_equip(w, a, e);
      break;
   case ACTION_REMOVE:
      status = action_remove(w, a, e);
      break;
   case ACTION_PUT:
      status = action_put(w, a, e);
      break;
   default:
      break;
   }

   if(act->id==ACTION_INVALID)
   {
      act->id = act_id;
      action_free(e);
      act->id = ACTION_INVALID;
   }

   return status;
}

void action_free(Entity *e)
{
   switch(e->action.id)
   {
   case ACTION_PATH:
      if(e->action.as.path.path!=NULL)
         RvR_free(e->action.as.path.path);
      break;
   default: break;
   }
}

void action_set_wait(Entity *e, uint32_t time)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_WAIT;
   e->action.cost = e->action_points;
   e->action.interrupt = 0;
   e->action.can_interrupt = 0;
}

void action_set_move(Entity *e, uint8_t dir)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_MOVE;
   e->action.cost = entity_move_cost(e);
   e->action.as.move.dir = dir;
   e->action.interrupt = 0;
   e->action.can_interrupt = 0;
}

void action_set_ascend(Entity *e)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_ASCEND;
   e->action.cost = entity_move_cost(e);
   e->action.interrupt = 0;
   e->action.can_interrupt = 0;
}

void action_set_descend(Entity *e)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_DESCEND;
   e->action.cost = entity_move_cost(e);
   e->action.interrupt = 0;
   e->action.can_interrupt = 0;
}

void action_set_attack(Entity *e, uint8_t dir)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_ATTACK;
   e->action.cost = 128;
   e->action.as.attack.dir = dir;
   e->action.interrupt = 0;
   e->action.can_interrupt = 0;
}

void action_set_path(Area *a, Entity *e, Point goal, uint32_t flags)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_PATH;
   e->action.cost = entity_move_cost(e);
   e->action.interrupt = 0;
   e->action.can_interrupt = 1;

   e->action.as.path.goal = goal;
   e->action.as.path.path = astar_path(a, e, goal, &e->action.as.path.len, flags);
   e->action.as.path.pos = 0;
   e->action.as.path.flags = flags;
}

void action_set_pickup(Area *a, Entity *e, Item_index index)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_PICKUP;
   e->action.cost = 128;
   e->action.interrupt = 0;
   e->action.can_interrupt = 1;

   e->action.as.pickup.item = index;
}

void action_set_drop(Area *a, Entity *e, Item_index index)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_DROP;
   e->action.cost = 128;
   e->action.interrupt = 0;
   e->action.can_interrupt = 1;

   e->action.as.drop.item = index;
}

void action_set_equip(Area *a, Entity *e, Item_index index)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_EQUIP;
   e->action.cost = 128;
   e->action.interrupt = 0;
   e->action.can_interrupt = 1;

   e->action.as.equip.item = index;
}

void action_set_remove(Area *a, Entity *e, Item_index index)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_REMOVE;
   e->action.cost = 128;
   e->action.interrupt = 0;
   e->action.can_interrupt = 1;

   e->action.as.remove.item = index;
}

void action_set_put(Area *a, Entity *e, Item_index index, Item_index container)
{
   if(e==NULL)
      return;

   action_free(e);

   e->action.id = ACTION_PUT;
   e->action.cost = 128;
   e->action.interrupt = 0;
   e->action.can_interrupt = 1;

   e->action.as.put.item = index;
   e->action.as.put.container = container;
}

void action_interrupt(Entity *e)
{
   e->action.interrupt = 1;
}

static int action_move(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   int status = entity_try_move(w, a, e, act->as.move.dir);
   return status==2?ACTION_LEFT_MAP:ACTION_FINISHED;
}

static int action_wait(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   return ACTION_FINISHED;
}

static int action_ascend(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   entity_try_ascend(a, e);
   return ACTION_FINISHED;
}

static int action_descend(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   entity_try_descend(a, e);
   return ACTION_FINISHED;
}

static int action_attack(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   Entity *target = area_entity_at(a, point_add_dir(e->pos, e->action.as.attack.dir), e);

   if(target==NULL)
      return ACTION_FINISHED;

   entity_hit(target, e, NULL, -1);
   //Damage random bodypart

   return ACTION_FINISHED;
}

static int action_path(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;
   if(act->as.path.path==NULL)
      return ACTION_FINISHED;

   int status = entity_try_move(w, a, e, act->as.path.path[act->as.path.pos]);
   if(!status)
   {
      if(act->as.path.path!=NULL)
         RvR_free(act->as.path.path);
      act->as.path.path = astar_path(a, e, act->as.path.goal, &act->as.path.len, act->as.path.flags);
      act->as.path.pos = 0;
      if(act->as.path.path==NULL)
         return ACTION_FINISHED;
      status = entity_try_move(w, a, e, act->as.path.path[act->as.path.pos]);
      if(!status)
         return ACTION_FINISHED;
   }

   if(status==2)
      return ACTION_LEFT_MAP;

   if(!act->interrupt&&!point_equal(e->pos, act->as.path.goal)&&act->as.path.pos<act->as.path.len - 1)
   {
      act->id = ACTION_PATH;
      act->cost = entity_move_cost(e);
      act->as.path.pos++;

      return ACTION_IN_PROGRESS;
   }

   return status==2?ACTION_LEFT_MAP:ACTION_FINISHED;
}

static int action_pickup(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   Item *it = item_index_try(act->as.pickup.item);

   if(it==NULL)
      return ACTION_FINISHED;

   if(!point_equal(it->pos, e->pos))
      return ACTION_FINISHED;

   entity_store_item(w, a, e, it);

   return ACTION_FINISHED;
}

static int action_drop(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   Item *it = item_index_try(act->as.drop.item);

   if(it==NULL)
      return ACTION_FINISHED;

   //Check if eligible
   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].type==ITEM_SLOT_GRASP)
         {
            Item *cur = e->body.parts[i].slots[j].it;
            for(; cur!=NULL; cur = cur->next)
            {
               if(cur==it)
               {
                  Item *ni = item_duplicate(w, it);
                  ni->pos = e->pos;
                  item_add(a, ni);
                  item_grid_add(a, ni);

                  item_free(it);

                  return ACTION_FINISHED;
               }
            }
         }

         for(Item *cur = e->body.parts[i].slots[j].it; cur!=NULL; cur = cur->next)
         {
            if(cur->def->tags & DEF_ITEM_SLOT_CONTAINER)
            {
               for(Item *con = cur->container.it; con!=NULL; con = con->next)
               {
                  if(con==it)
                  {
                     Item *ni = item_duplicate(w, it);
                     ni->pos = e->pos;
                     item_add(a, ni);
                     item_grid_add(a, ni);

                     item_free(it);

                     return ACTION_FINISHED;
                  }
               }
            }
         }
      }
   }

   return ACTION_FINISHED;
}

static int action_equip(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   Item *it = item_index_try(act->as.equip.item);

   if(it==NULL)
      return ACTION_FINISHED;

   if(!entity_can_equip(w, a, e, it, 1))
      return ACTION_FINISHED;

   //Check if eligible
   for(int i = 0; i<e->body.part_count; i++)
   {
      if(e->body.parts[i].hp<=0)
         continue;

      for(int j = 0; j<e->body.parts[i].slot_count; j++)
      {
         if(e->body.parts[i].slots[j].type==ITEM_SLOT_GRASP)
         {
            Item *cur = e->body.parts[i].slots[j].it;
            for(; cur!=NULL; cur = cur->next)
            {
               if(cur==it)
               {
                  entity_equip(w, a, e, it);
                  //Item *inv = item_duplicate(w,it);
                  //item_free(it);
                  //inv->prev_next = &e->body.parts[i].slots[j].it;
                  //inv->next = e->body.parts[i].slots[j].it;
                  //e->body.parts[i].slots[j].it = inv;

                  return ACTION_FINISHED;
               }
            }
         }

         for(Item *cur = e->body.parts[i].slots[j].it; cur!=NULL; cur = cur->next)
         {
            if(cur->def->tags & DEF_ITEM_SLOT_CONTAINER)
            {
               for(Item *con = cur->container.it; con!=NULL; con = con->next)
               {
                  if(con==it)
                  {
                     entity_equip(w, a, e, it);
                     return ACTION_FINISHED;
                  }
               }
            }
         }
      }
   }

   return ACTION_FINISHED;
}

static int action_remove(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   Item *it = item_index_try(act->as.remove.item);

   entity_remove(w, a, e, it);

   return ACTION_FINISHED;
}

static int action_put(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   Item *it = item_index_try(act->as.put.item);
   Item *con = item_index_try(act->as.put.container);

   entity_put(w, a, e, it, con);

   return ACTION_FINISHED;
}
//-------------------------------------
