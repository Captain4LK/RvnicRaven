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
#include "entity_documented.h"
#include "area.h"
#include "tile.h"
#include "ai.h"

#include "player.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Entity *entity_pool;
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

   uint64_t id = w->next_eid++;
   memset(n, 0, sizeof(*n));
   n->next = NULL;
   n->prev_next = NULL;
   n->g_next = NULL;
   n->g_prev_next = NULL;
   n->id = id;

   return n;
}

void entity_free(Entity *e)
{
   if(e == NULL)
      return;

   *e->prev_next = e->next;
   if(e->next != NULL)
      e->next->prev_next = e->prev_next;

   e->next = entity_pool;
   entity_pool = e;
}

void entity_remove(Entity *e)
{
   if(e == NULL)
      return;

   entity_grid_remove(e);

   e->id = UINT64_MAX;
   e->removed = 1;
}

void entity_add(Area *a, Entity *e)
{
   if(e==NULL)
      return;

   e->prev_next = &a->entities;
   if(a->entities!=NULL)
      a->entities->prev_next = &e->next;
   e->next = a->entities;
   a->entities = e;
}

void entity_grid_add(Area *a, Entity *e)
{
   if(e==NULL)
      return;
   if(e->x<0||e->y<0||e->z<0)
      return;
   if(e->x>=a->dimx * 32||e->y>=a->dimy * 32||e->z>=a->dimz * 32)
      return;

   int gx = e->x / 8;
   int gy = e->y / 8;
   int gz = e->z / 8;
   size_t g_index = gz * (a->dimx * 4) * (a->dimy * 4) + gy * (a->dimx * 4) + gx;
   e->g_prev_next = &a->entity_grid[g_index];
   if(a->entity_grid[g_index]!=NULL)
      a->entity_grid[g_index]->prev_next = &e->g_next;
   e->g_next = a->entity_grid[g_index];
   a->entity_grid[g_index] = e;
}

void entity_update_pos(Area *a, Entity *e, int16_t x, int16_t y, int16_t z)
{
   if(e==NULL)
      return;
   if(x<0||y<0||z<0)
      return;
   if(x>=a->dimx * 32||y>=a->dimy * 32||z>=a->dimz * 32)
      return;

   entity_grid_remove(e);
   e->x = x;
   e->y = y;
   e->z = z;
   entity_grid_add(a, e);
}

void entity_grid_remove(Entity *e)
{
   *e->g_prev_next = e->g_next;
   if(e->g_next != NULL)
      e->g_next->prev_next = e->g_prev_next;
}

int entity_pos_valid(Area *a, Entity *e, int x, int y, int z)
{
   if(e==NULL)
      return 0;

   if(x<0||y<0||z<0)
      return 0;

   if(x>=a->dimx * 32||y>=a->dimy * 32||z>=a->dimz * 32)
      return 0;

   uint32_t block = area_tile(a, x, y, z);
   uint32_t floor = area_tile(a, x, y, z + 1);
   if(!tile_has_wall(block)&&(tile_has_floor(floor)||tile_is_slope(floor)))
      return 1;
   return 0;
}

unsigned entity_try_move(World *w, Area *a, Entity *e, int dir)
{
   if(e == NULL)
      return 0;

   const int16_t dirs[8][2] =
   {
      { 1, 0 },
      { 0, 1 },
      { -1, 0 },
      { 0, -1 },

      //Diagonal
      { -1, 1 },
      { -1, -1 },
      { 1, -1 },
      { 1, 1 },
   };

   //Leave map
   //Update DocEnt if entity is documented
   int nx = e->x + dirs[dir][0];
   int ny = e->y + dirs[dir][1];
   if(nx<0||ny<0||nx>=a->dimx * 32||ny>=a->dimy * 32)
   {
      if(entity_is_docent(e))
      {
         e->x += dirs[dir][0];
         e->y += dirs[dir][1];
         docent_from_entity(w, a, e);
         e->x -= dirs[dir][0];
         e->y -= dirs[dir][1];
      }

      entity_remove(e);
      return 2;
   }

   if(entity_pos_valid(a, e, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z))
   {
      entity_update_pos(a, e, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z);
      return 1;
   }

   //Moving up slopes
   if(tile_is_slope(area_tile(a, e->x, e->y, e->z))&&
      tile_has_wall(area_tile(a, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z))&&
      !tile_has_wall(area_tile(a, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z - 1)))
   {
      entity_update_pos(a, e, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z - 1);
      return 1;
   }

   //Moving down slopes
   if(tile_is_slope(area_tile(a, e->x, e->y, e->z + 1))&&
      !tile_has_wall(area_tile(a, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z + 1))&&
      !tile_has_wall(area_tile(a, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z)))
   {
      entity_update_pos(a, e, e->x + dirs[dir][0], e->y + dirs[dir][1], e->z + 1);
      return 1;
   }

   return 0;
}

unsigned entity_try_ascend(Area *a, Entity *e)
{
   if(e==NULL)
      return 0;

   if(entity_pos_valid(a, e, e->x, e->y, e->z - 1))
   {
      entity_update_pos(a, e, e->x, e->y, e->z - 1);
      return 1;
   }

   return 0;
}

unsigned entity_try_descend(Area *a, Entity *e)
{
   if(e==NULL)
      return 0;

   if(entity_pos_valid(a, e, e->x, e->y, e->z + 1))
   {
      entity_update_pos(a, e, e->x, e->y, e->z + 1);
      return 1;
   }

   return 0;
}

void entity_think(World *w, Area *a, Entity *e)
{
   ai_think(w, a, e);
}

int entity_move_cost(Entity *e)
{
   return 184;
}
//-------------------------------------
