/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "../config.h"
#include "../card.h"
#include "../sprite.h"
#include "../entity.h"
#include "../ai.h"
#include "../game.h"
#include "../map.h"
#include "ai_elevator.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   uint8_t state;
   int32_t tick;
   RvR_fix16 top;
   RvR_fix16 bottom;
}AI_elevator_state;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void ai_elevator_init(Entity *e, const uint32_t extra[3])
{
   AI_elevator_state *state = RvR_malloc(sizeof(*state),"AI:elevator state");
   e->ai_data = state;
   e->sprite = -1;

   state->state = 0;
   state->bottom = extra[0];
   state->top = extra[1];
   state->tick = 0;
}

void ai_elevator_free(Entity *e)
{
   RvR_free(e->ai_data);
}

void ai_elevator_run(Entity *e)
{
   AI_elevator_state *state = e->ai_data;

   switch(state->state)
   {
   case 0: //Rise
      {
         RvR_fix16 z = RvR_ray_map_floor_height_at(map_current(),e->x/65536,e->y/65536);
         z = RvR_min(state->top,z+48*64);
         RvR_ray_map_floor_height_set(map_current(),e->x/65536,e->y/65536,z);

         if(z>=state->top)
         {
            state->state = 3;
            state->tick = 30;
         }
      }
      break;
   case 1: //Lower
      {
         RvR_fix16 z = RvR_ray_map_floor_height_at(map_current(),e->x/65536,e->y/65536);
         z = RvR_max(state->bottom,z-48*64);
         RvR_ray_map_floor_height_set(map_current(),e->x/65536,e->y/65536,z);

         if(z<=state->bottom)
         {
            state->state = 2;
            state->tick = 30;
         }
      }
      break;
   case 2: //Wait bottom
      {
         if(state->tick--<=0)
            state->state = 0;
      }
      break;
   case 3: //Wait top 
      {
         if(state->tick--<=0)
            state->state = 1;
      }
      break;
   }
}
//-------------------------------------
