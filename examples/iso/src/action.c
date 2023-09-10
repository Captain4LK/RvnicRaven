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
#include "action.h"
#include "entity.h"
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
//-------------------------------------

//Function implementations

int action_do(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   if(act->remaining<=e->action_points)
   {
      e->action_points -= act->remaining;
      act->remaining = 0;
   }
   else
   {
      act->remaining -= e->action_points;
      e->action_points = 0;
   }

   int status = 0;

   switch(e->action.id)
   {
   case ACTION_WAIT:
      status = action_wait(w,a, e);
      break;
   case ACTION_MOVE:
      status = action_move(w,a, e);
      break;
   case ACTION_ASCEND:
      status = action_ascend(w,a, e);
      break;
   case ACTION_DESCEND:
      status = action_descend(w,a, e);
      break;
   default:
      break;
   }

   if(act->remaining==0)
   {
      act->id = ACTION_INVALID;
   }

   return status;
}

void action_set_wait(Entity *e, uint32_t time)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_WAIT;
   e->action.remaining = e->action_points;
   e->action.can_interrupt = 1;
}

void action_set_move(Entity *e, uint8_t dir)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_MOVE;
   e->action.remaining = entity_move_cost(e);
   e->action.as.move.dir = dir;
   e->action.can_interrupt = 1;
}

void action_set_ascend(Entity *e)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_ASCEND;
   e->action.remaining = entity_move_cost(e);
   e->action.can_interrupt = 1;
}

void action_set_descend(Entity *e)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_DESCEND;
   e->action.remaining = entity_move_cost(e);
   e->action.can_interrupt = 1;
}

static int action_move(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;
   if(act->remaining==0)
   {
      int status = entity_try_move(w,a, e, act->as.move.dir);
      return status==2?ACTION_LEFT_MAP:ACTION_FINISHED;
   }

   return ACTION_IN_PROGRESS;
}

static int action_wait(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;

   return 0;
}

static int action_ascend(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;
   if(act->remaining==0)
   {
      entity_try_ascend(a,e);
      return ACTION_FINISHED;
   }

   return ACTION_IN_PROGRESS;
}

static int action_descend(World *w, Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;
   if(act->remaining==0)
   {
      entity_try_descend(a,e);
      return ACTION_FINISHED;
   }

   return ACTION_IN_PROGRESS;
}
//-------------------------------------
