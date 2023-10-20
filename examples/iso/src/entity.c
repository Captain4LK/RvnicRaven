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
#include "log.h"
#include "body.h"

#include "player.h"
#include "sprite.h"
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
   n->action.id = ACTION_INVALID;
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
   if(e->pos.x<0||e->pos.y<0||e->pos.z<0)
      return;
   if(e->pos.x>=a->dimx * 32||e->pos.y>=a->dimy * 32||e->pos.z>=a->dimz * 32)
      return;

   int gx = e->pos.x / 8;
   int gy = e->pos.y / 8;
   int gz = e->pos.z / 8;
   size_t g_index = gz * (a->dimx * 4) * (a->dimy * 4) + gy * (a->dimx * 4) + gx;
   e->g_prev_next = &a->entity_grid[g_index];
   if(a->entity_grid[g_index]!=NULL)
      a->entity_grid[g_index]->g_prev_next = &e->g_next;
   e->g_next = a->entity_grid[g_index];
   a->entity_grid[g_index] = e;
}

void entity_update_pos(Area *a, Entity *e, Point new_pos)
{
   if(e==NULL)
      return;
   if(new_pos.x<0||new_pos.y<0||new_pos.z<0)
      return;
   if(new_pos.x>=a->dimx * 32||new_pos.y>=a->dimy * 32||new_pos.z>=a->dimz * 32)
      return;

   entity_grid_remove(e);
   e->pos = new_pos;
   entity_grid_add(a, e);
}

void entity_grid_remove(Entity *e)
{
   if(e==NULL)
      return;

   *e->g_prev_next = e->g_next;
   if(e->g_next != NULL)
      e->g_next->g_prev_next = e->g_prev_next;
}

int entity_pos_valid(Area *a, Entity *e, Point pos)
{
   if(e==NULL)
      return 0;

   if(pos.x<0||pos.y<0||pos.z<0)
      return 0;

   if(pos.x>=a->dimx * 32||pos.y>=a->dimy * 32||pos.z>=a->dimz * 32)
      return 0;

   uint32_t block = area_tile(a, pos);
   uint32_t floor = area_tile(a, point(pos.x,pos.y,pos.z+1));
   if(!tile_has_wall(block)&&(tile_has_floor(floor)||tile_is_slope(floor)))
      return 1;
   return 0;
}

unsigned entity_try_move(World *w, Area *a, Entity *e, uint8_t dir)
{
   if(e == NULL)
      return 0;

   //Leave map
   //Update DocEnt if entity is documented
   Point n = point_add_dir(e->pos,dir);
   if(n.x<0||n.y<0||n.x>=a->dimx * 32||n.y>=a->dimy * 32)
   {
      if(entity_is_docent(e))
      {
         e->pos = point_add_dir(e->pos,dir);
         docent_from_entity(w, a, e);
         e->pos = point_sub_dir(e->pos,dir);
      }

      entity_remove(e);
      return 2;
   }

   if(entity_pos_valid(a, e, n))
   {
      entity_update_pos(a, e, n);
      return 1;
   }

   //Moving up slopes
   if(tile_is_slope(area_tile(a, e->pos))&&
      tile_has_wall(area_tile(a, n))&&
      !tile_has_wall(area_tile(a, point(n.x,n.y,n.z-1))))
   {
      entity_update_pos(a, e, point(n.x,n.y,n.z-1));
      return 1;
   }

   //Moving down slopes
   if(tile_is_slope(area_tile(a, point(e->pos.x, e->pos.y, e->pos.z + 1)))&&
      !tile_has_wall(area_tile(a, point(n.x,n.y,n.z+1)))&&
      !tile_has_wall(area_tile(a, n)))
   {
      entity_update_pos(a, e, point(n.x,n.y,n.z+1));
      return 1;
   }

   return 0;
}

unsigned entity_try_ascend(Area *a, Entity *e)
{
   if(e==NULL)
      return 0;

   if(entity_pos_valid(a, e, point(e->pos.x, e->pos.y, e->pos.z - 1)))
   {
      entity_update_pos(a, e, point(e->pos.x, e->pos.y, e->pos.z - 1));
      return 1;
   }

   return 0;
}

unsigned entity_try_descend(Area *a, Entity *e)
{
   if(e==NULL)
      return 0;

   if(entity_pos_valid(a, e, point(e->pos.x, e->pos.y, e->pos.z + 1)))
   {
      entity_update_pos(a, e, point(e->pos.x, e->pos.y, e->pos.z + 1));
      return 1;
   }

   return 0;
}

void entity_think(World *w, Area *a, Entity *e)
{
   ai_think(w, a, e);
}

void entity_turn(World *w, Area *a, Entity *e)
{
   e->fatigue_next--;
   e->hunger_next--;
   
   if(e->fatigue_next<=0)
   {
      e->fatigue++;
      e->fatigue_next = 300;
   }

   if(e->hunger_next<=0)
   {
      e->hunger++;
      e->hunger_next = 300;
   }

}

int entity_move_cost(Entity *e)
{
   return 184;
}

void entity_from_def(Entity *e, const EntityDef *def, int female)
{
   if(female)
      body_from_def(&e->body,def->female_body);
   else
      body_from_def(&e->body,def->male_body);
}

void entity_sprite_create(Entity *e)
{
   e->sprite = sprite_new(e);
   sprite_clear(e->sprite,0);

   if(e->body.def==NULL)
   {
      sprite_draw_sprite(e->sprite,16384,0,0,0,0,32,32);
      return;
   }
   uint16_t sheet = e->body.def->sprite_sheet;
   for(int i = 0;i<e->body.part_count;i++)
   {
      const Bodypart *bp = &e->body.parts[i];
      int16_t index = bp->def->sprite_index;
      if(index<0)
         continue;
      sprite_draw_sprite(e->sprite,sheet,0,0,index*32,0,32,36);
   }
}

void entity_hit(Entity *e, Entity *src, Item *weapon, int16_t body_part)
{
   if(e==NULL)
      return;

   if(body_part==-1)
   {
      int part_count = 0;
      for(int i = 0;i<e->body.part_count;i++)
         if(e->body.parts[i].hp>0)
            part_count++;
   }

   //if(body_part<0)
      //return;

   e->body.parts[0].hp-=20;
   puts("DAMAGE");
}
//-------------------------------------
