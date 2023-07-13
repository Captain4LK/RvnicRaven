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
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static int action_move(Area *a, Entity *e);
static int action_wait(Area *a, Entity *e);
static int action_ascend(Area *a, Entity *e);
static int action_descend(Area *a, Entity *e);
//-------------------------------------

//Function implementations

void action_do(Area *a, Entity *e)
{
   int time = 0;

   switch(e->action.id)
   {
   case ACTION_WAIT:
      time = action_wait(a,e);
      break;
   case ACTION_MOVE:
      time = action_move(a,e);
      break;
   case ACTION_ASCEND:
      time = action_ascend(a,e);
      break;
   case ACTION_DESCEND:
      time = action_descend(a,e);
      break;
   default:
      return;
      break;
   }

   e->turn_next = (time+e->turn_rem)/128;
   e->turn_rem = (time+e->turn_rem)%128;
}

void action_set_wait(Entity *e, uint32_t time)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_WAIT;
   e->action.as.wait.time = time;
}

void action_set_move(Entity *e, uint8_t dir)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_MOVE;
   e->action.as.move.dir = dir;
}

void action_set_ascend(Entity *e)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_ASCEND;
}

void action_set_descend(Entity *e)
{
   if(e==NULL)
      return;

   e->action.id = ACTION_DESCEND;
}

static int action_move(Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = !entity_try_move(a, e, act->as.move.dir);
   act->id = ACTION_INVALID;

   return 1152;
}

static int action_wait(Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = 0;
   act->id = ACTION_INVALID;

   return act->as.wait.time;
}

static int action_ascend(Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = !entity_try_ascend(a,e);
   act->id = ACTION_INVALID;

   return 1152;
}

static int action_descend(Area *a, Entity *e)
{
   Action *act = &e->action;
   act->status = !entity_try_descend(a,e);
   act->id = ACTION_INVALID;

   return 1152;
}
//-------------------------------------
