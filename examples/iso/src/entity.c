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
#include "world_defs.h"
#include "entity.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Entity *entity_pool;
static Entity_cube *entity_cube_pool;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Entity *entity_new(World *w)
{
   if(entity_pool == NULL)
   {
      Entity *ne = RvR_malloc(sizeof(*ne) * 256, "entity pool");
      memset(ne, 0, sizeof(*ne) * 256);

      for(int i = 0; i < 256; i++)
         ne[i].id = UINT64_MAX;
      for(int i = 0; i < 256 - 1; i++)
         ne[i].next = &ne[i + 1];
      entity_pool = &ne[0];
   }

   Entity *n = entity_pool;
   entity_pool = n->next;

   uint32_t id = w->next_id++;
   memset(n, 0, sizeof(*n));
   n->next = NULL;
   n->prev_next = NULL;
   n->id = id;

   return n;
}

void entity_free(Entity *e)
{
   if(e == NULL)
      return;

   /*switch(e->ai_type)
   {
   case AI_INVALID:
      break;
   case AI_QUADRUPED:
      quadruped_free(e);
      break;
   }

   if(e->path != NULL)
      RvR_free(e->path);*/

   *e->prev_next = e->next;
   if(e->next != NULL)
      e->next->prev_next = e->prev_next;

   e->next = entity_pool;
   entity_pool = e;
}

Entity_cube *entity_cube_new()
{
   if(entity_cube_pool == NULL)
   {
      Entity_cube *ne = RvR_malloc(sizeof(*ne) * 256, "entity_cube pool");
      memset(ne, 0, sizeof(*ne) * 256);

      for(int i = 0; i < 256 - 1; i++)
         ne[i].next = &ne[i + 1];
      entity_cube_pool = &ne[0];
   }

   Entity_cube *n = entity_cube_pool;
   entity_cube_pool = n->next;

   memset(n, 0, sizeof(*n));
   n->next = NULL;
   n->prev_next = NULL;

   return n;
}

void entity_cube_free(Entity_cube *e)
{
   if(e==NULL)
      return;

   *e->prev_next = e->next;
   if(e->next != NULL)
      e->next->prev_next = e->prev_next;

   e->next = entity_cube_pool;
   entity_cube_pool = e;
}

void entity_remove(Entity *e)
{
   if(e == NULL)
      return;

   e->id = UINT64_MAX;
   e->removed = 1;
}
//-------------------------------------
