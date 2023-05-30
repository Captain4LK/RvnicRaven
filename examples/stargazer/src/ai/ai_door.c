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
#include "../player.h"
#include "../grid.h"
#include "../map.h"
#include "ai_door.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   uint8_t key;
   uint8_t state;
   int32_t tick;
   RvR_fix16 top;
   RvR_fix16 bottom;
}AI_door_state;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void ai_door_init(Entity *e, const uint32_t extra[3])
{
   AI_door_state *state = RvR_malloc(sizeof(*state),"AI:door state");
   e->ai_data = state;
   e->sprite = -1;

   state->key = extra[2];
   state->top = extra[1];
   state->bottom = extra[0];
   state->state = 0;
   state->tick = 0;
}

void ai_door_free(Entity *e)
{
   RvR_free(e->ai_data);
}

void ai_door_run(Entity *e)
{
   AI_door_state *state = e->ai_data;

   //Check if entity in way
   int x = e->x/65536;
   int y = e->y/65536;
   Grid_square *s = grid_square(x,y);
   if(state->state==0&&s!=NULL)
   {
      Grid_entity *ge = s->entities;
      while(ge!=NULL)
      {
         if(!ge->ent->removed&&ge->ent->col_radius>0)
         {
            state->state = 1;
            break;
         }
         ge = ge->next;
      }
   }

   switch(state->state)
   {
   case 0: //Rise
      {

         RvR_fix16 z = RvR_ray_map_floor_height_at(map_current(),e->x/65536,e->y/65536);
         z = RvR_min(state->top,z+48*64);
         RvR_ray_map_floor_height_set(map_current(),e->x/65536,e->y/65536,z);

         if(z>=state->top)
            state->state = 3;
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
            state->tick = 60;
         }
      }
      break;
   case 2: //Still bottom
      {
         if(state->tick--<=0)
            state->state = 0;
      }
      break;
   case 3: //Still top 
      break;
   }
}

void ai_door_on_use(Entity *e, Entity *trigger)
{
   AI_door_state *state = e->ai_data;

   //TODO: check if has key

   //Check position
   int ex = e->x/65536;
   int ey = e->y/65536;
   int tx = trigger->x/65536;
   int ty = trigger->y/65536;
   if(RvR_min(RvR_abs(ex-tx),RvR_abs(ey-ty))!=0)
      return;

   //Check distance
   RvR_fix16 dist_x = RvR_abs(trigger->x-e->x);
   RvR_fix16 dist_y = RvR_abs(trigger->y-e->y);
   RvR_fix16 dist = RvR_max(dist_x,dist_y);
   if(dist>81920)
      return;

   //Check fov
   RvR_fix16 tdirx = RvR_fix16_cos(trigger->direction);
   RvR_fix16 tdiry = RvR_fix16_sin(trigger->direction);
   RvR_fix16 tox = e->x-trigger->x;
   RvR_fix16 toy = e->y-trigger->y;
   //RvR_fix22 len = RvR_non_zero(RvR_fix22_sqrt((tox*tox+toy*toy)/1024));
   RvR_fix16 len = RvR_non_zero(RvR_fix16_sqrt(RvR_fix16_mul(tox,tox)+RvR_fix16_mul(toy,toy)));
   tox = RvR_fix16_div(tox,len);
   toy = RvR_fix16_div(toy,len);
   //RvR_fix22 angle = (tdir.x*to.x+tdir.y*to.y)/1024;
   RvR_fix16 angle = RvR_fix16_mul(tdirx,tox)+RvR_fix16_mul(tdiry,toy);
   //if(angle<724)
   if(angle<16384)
      return;

   state->state = 1;
}
//-------------------------------------
