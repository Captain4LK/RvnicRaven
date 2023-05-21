/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//TODO: generalise to use thing instead of entities?
//store cards/particles in seperate linked lists?

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../RvR/RvnicRaven.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "map.h"
#include "grid.h"
#include "ai.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Grid grid = {0};
static uint32_t grid_counter = 0;

static Grid_entity *grid_entity_pool = NULL;
static Grid_card *grid_card_pool = NULL;
//-------------------------------------

//Function prototypes
static Grid_entity *grid_entity_new();
static void grid_entity_free(Grid_entity *e);
static Grid_card *grid_card_new();
static void grid_card_free(Grid_card *c);
//-------------------------------------

//Function implementations

void grid_build()
{
   if(grid.grid!=NULL)
   {
      for(int i = 0;i<grid.width*grid.height;i++)
      {
         Grid_entity *e = grid.grid[i].entities;
         while(e!=NULL)
         {
            Grid_entity *next = e->next;
            grid_entity_free(e);

            e = next;
         }
      }

      RvR_free(grid.grid);
   }

   RvR_ray_map *map = RvR_ray_map_get();
   grid.width = map->width;
   grid.height = map->height;
   grid.grid = RvR_malloc(sizeof(*grid.grid)*grid.width*grid.height);

   //Calculate whether a cell is solid
   for(int i = 0;i<grid.width*grid.height;i++)
   {
      RvR_fix22 floor = map->floor[i];
      RvR_fix22 ceiling = map->ceiling[i];

      grid.grid[i].blocked = floor>=ceiling;
      grid.grid[i].entities = NULL;
   }

   //Add entities
   Entity *e = entities;
   while(e!=NULL)
   {
      grid_entity_add(e);
      e = e->next;
   }
}

void grid_entity_remove(Entity *e)
{
   int l = (e->pos.x-e->col_radius)/1024;
   int r = (e->pos.x+e->col_radius)/1024;
   int t = (e->pos.y-e->col_radius)/1024;
   int b = (e->pos.y+e->col_radius)/1024;

   for(int y = t;y<=b;y++)
   {
      for(int x = l;x<=r;x++)
      {
         if(x<0||x>=grid.width||y<0||y>=grid.height)
            continue;

         Grid_entity *ge = grid.grid[y*grid.width+x].entities;
         while(ge!=NULL)
         {
            Grid_entity *next = ge->next;

            if(ge->ent==e)
               grid_entity_free(ge);

            ge = next;
         }
      }
   }
}

void grid_entity_add(Entity *e)
{
   int l = (e->pos.x-e->col_radius)/1024;
   int r = (e->pos.x+e->col_radius)/1024;
   int t = (e->pos.y-e->col_radius)/1024;
   int b = (e->pos.y+e->col_radius)/1024;

   for(int y = t;y<=b;y++)
   {
      for(int x = l;x<=r;x++)
      {
         if(x<0||x>=grid.width||y<0||y>=grid.height)
            continue;

         Grid_entity *ge = grid_entity_new();
         ge->ent = e;
         ge->prev_next = &grid.grid[y*grid.width+x].entities;
         if(grid.grid[y*grid.width+x].entities!=NULL)
            grid.grid[y*grid.width+x].entities->prev_next = &ge->next;
         ge->next = grid.grid[y*grid.width+x].entities;
         grid.grid[y*grid.width+x].entities = ge;
      }
   }
}

void grid_entity_update_pos(Entity *e, RvR_vec3 new_pos)
{
   if(e->pos.x!=new_pos.x&&e->pos.y==new_pos.y)
   {
      e->pos = new_pos;
      return;
   }

   grid_entity_remove(e);
   e->pos = new_pos;
   grid_entity_add(e);
}

void grid_card_remove(Card *c)
{
   int l = (c->pos.x-CARD_RADIUS)/1024;
   int r = (c->pos.x+CARD_RADIUS)/1024;
   int t = (c->pos.y-CARD_RADIUS)/1024;
   int b = (c->pos.y+CARD_RADIUS)/1024;

   for(int y = t;y<=b;y++)
   {
      for(int x = l;x<=r;x++)
      {
         if(x<0||x>=grid.width||y<0||y>=grid.height)
            continue;

         Grid_card *gc = grid.grid[y*grid.width+x].cards;
         while(gc!=NULL)
         {
            Grid_card *next = gc->next;

            if(gc->card==c)
               grid_card_free(gc);

            gc = next;
         }
      }
   }
}

void grid_card_add(Card *c)
{
   int l = (c->pos.x-CARD_RADIUS)/1024;
   int r = (c->pos.x+CARD_RADIUS)/1024;
   int t = (c->pos.y-CARD_RADIUS)/1024;
   int b = (c->pos.y+CARD_RADIUS)/1024;

   for(int y = t;y<=b;y++)
   {
      for(int x = l;x<=r;x++)
      {
         if(x<0||x>=grid.width||y<0||y>=grid.height)
            continue;

         Grid_card *gc = grid_card_new();
         gc->card = c;
         gc->prev_next = &grid.grid[y*grid.width+x].cards;
         if(grid.grid[y*grid.width+x].cards!=NULL)
            grid.grid[y*grid.width+x].cards->prev_next = &gc->next;
         gc->next = grid.grid[y*grid.width+x].cards;
         grid.grid[y*grid.width+x].cards = gc;
      }
   }
}

void grid_card_update_pos(Card *c, RvR_vec3 new_pos)
{
   if(c->pos.x!=new_pos.x&&c->pos.y==new_pos.y)
   {
      c->pos = new_pos;
      return;
   }

   grid_card_remove(c);
   c->pos = new_pos;
   grid_card_add(c);
}

void grid_entity_use(Entity *e)
{
   int l = (e->pos.x-e->col_radius)/1024-1;
   int r = (e->pos.x+e->col_radius)/1024+1;
   int t = (e->pos.y-e->col_radius)/1024-1;
   int b = (e->pos.y+e->col_radius)/1024+1;

   uint32_t count = grid_counter_next();

   for(int y = t;y<=b;y++)
   {
      for(int x = l;x<=r;x++)
      {
         if(x<0||x>=grid.width||y<0||y>=grid.height)
            continue;

         Grid_entity *ge = grid.grid[y*grid.width+x].entities;
         while(ge!=NULL)
         {
            if(ge->ent->removed)
               goto next;
            if(ge->ent->grid_counter==count)
               goto next;
            ge->ent->grid_counter = count;
            ai_on_use(ge->ent,e);
            
next:
            ge = ge->next;
         }
      }
   }
}

Grid_square *grid_square(int x, int y)
{
   if(x<0||x>=grid.width||y<0||y>=grid.height)
      return NULL;

   return &grid.grid[y*grid.width+x];
}

uint32_t grid_counter_next()
{
   //TODO: overflow protection
   return ++grid_counter;
}

static Grid_entity *grid_entity_new()
{
   if(grid_entity_pool==NULL)
   {
      Grid_entity *ne = RvR_malloc(sizeof(*ne)*256);
      memset(ne,0,sizeof(*ne)*256);
      
      for(int i = 0;i<256-1;i++)
         ne[i].next = &ne[i+1];
      grid_entity_pool = &ne[0];
   }

   Grid_entity *n = grid_entity_pool;
   grid_entity_pool = n->next;
   n->next = NULL;
   n->prev_next = NULL;

   return n;
}

static void grid_entity_free(Grid_entity *e)
{
   if(e==NULL)
      return;

   *e->prev_next = e->next;
   if(e->next!=NULL)
      e->next->prev_next = e->prev_next;

   e->next = grid_entity_pool; 
   grid_entity_pool = e;
}

static Grid_card *grid_card_new()
{
   if(grid_card_pool==NULL)
   {
      Grid_card *nc = RvR_malloc(sizeof(*nc)*256);
      memset(nc,0,sizeof(*nc)*256);
      
      for(int i = 0;i<256-1;i++)
         nc[i].next = &nc[i+1];
      grid_card_pool = &nc[0];
   }

   Grid_card *c = grid_card_pool;
   grid_card_pool = c->next;
   c->next = NULL;
   c->prev_next = NULL;

   return c;
}

static void grid_card_free(Grid_card *c)
{
   if(c==NULL)
      return;

   *c->prev_next = c->next;
   if(c->next!=NULL)
      c->next->prev_next = c->prev_next;

   c->next = grid_card_pool; 
   grid_card_pool = c;
}
//-------------------------------------
