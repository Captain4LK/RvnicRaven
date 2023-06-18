/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "ai.h"

#include "ai/ai_elevator.h"
#include "ai/ai_door.h"
#include "ai/ai_deco.h"
#include "ai/ai_block.h"
#include "ai/ai_officer.h"
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

void ai_run(Entity *e)
{
   if(e==NULL||e->removed)
      return;

   switch(e->ai_type)
   {
   case 0: break; //no ai
   case 1: ai_elevator_run(e); break;
   case 2: ai_door_run(e); break;
   case 3: ai_deco_run(e); break;
   case 4: ai_block_run(e); break;
   case 5: ai_officer_run(e); break;
   }
}

void ai_on_use(Entity *e, Entity *trigger)
{
   if(e==NULL||e->removed)
      return;

   switch(e->ai_type)
   {
   case 0: break; //no ai
   case 2: ai_door_on_use(e, trigger); break;
   }
}

void ai_free(Entity *e)
{
   if(e==NULL)
      return;

   switch(e->ai_type)
   {
   case 0: break; //no ai
   case 1: ai_elevator_free(e); break;
   case 2: ai_door_free(e); break;
   case 3: ai_deco_free(e); break;
   case 4: ai_block_free(e); break;
   case 5: ai_officer_free(e); break;
   }
}

void ai_init(Entity *e, uint32_t ai_type, const uint32_t extra[3])
{
   if(e==NULL||e->removed)
      return;

   e->ai_type = ai_type;

   switch(e->ai_type)
   {
   case 0: break; //no ai
   case 1: ai_elevator_init(e, extra); break;
   case 2: ai_door_init(e, extra); break;
   case 3: ai_deco_init(e, extra); break;
   case 4: ai_block_init(e, extra); break;
   case 5: ai_officer_init(e, extra); break;
   }
}
//-------------------------------------
