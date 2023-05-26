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
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
#include "ai.h"
#include "map.h"
#include "grid.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static RvR_ray_map *map = NULL;
//-------------------------------------

//Function prototypes
static uint32_t ai_from_tex(uint16_t tex);
//-------------------------------------

//Function implementations

void map_load(uint16_t id)
{
   if(map!=NULL)
      RvR_ray_map_free(map);
   map = RvR_ray_map_load(id);
}

void map_init()
{
   for(int i = 0;i<map->sprite_count;i++)
   {
      RvR_ray_map_sprite *s = map->sprites+i;

      uint32_t extra[3];
      extra[0] = s->extra0;
      extra[1] = s->extra1;
      extra[2] = s->extra2;

      Entity *e = entity_new();
      e->x = s->x;
      e->y = s->y;
      e->z = s->z;
      e->direction = s->direction;
      e->sprite = s->texture;
      entity_add(e);

      //Init ai
      ai_init(e,ai_from_tex(s->texture),extra);

      //Player
      if(s->texture==36864)
      {
         Card *c = card_new();
         c->x = e->x;
         c->y = e->y;
         c->z = e->z;
         card_add(c);
         player.entity = e;
         player_create_new();
      }
   }

   grid_build();
}

static uint32_t ai_from_tex(uint16_t tex)
{
   switch(tex)
   {
   case 36865: return 1; //elevator
   case 36866: return 2; //door
   case 36867: return 4; //blocker
   default: return 3; //default is decoration
   }

   return 0;
}

RvR_ray_map *map_current()
{
   return map;
}
//-------------------------------------
