/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "item.h"
#include "sprite.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Item *item_pool = NULL;
static char item_name_buffer[512];
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

Item *item_new(World *w)
{
   if(item_pool == NULL)
   {
      Item *ni = RvR_malloc(sizeof(*ni) * 256, "item pool");
      memset(ni, 0, sizeof(*ni) * 256);

      for(int i = 0; i < 256; i++)
         ni[i].id = UINT64_MAX;
      for(int i = 0; i < 256 - 1; i++)
         ni[i].next = &ni[i + 1];
      item_pool = &ni[0];
   }

   Item *n = item_pool;
   item_pool = n->next;

   uint64_t id = w->next_iid++;
   memset(n, 0, sizeof(*n));
   n->next = NULL;
   n->prev_next = NULL;
   n->g_next = NULL;
   n->g_prev_next = NULL;
   n->id = id;
   n->container.it = NULL;
   n->container.type = ITEM_SLOT_NONE;

   return n;
}

void item_free(Item *i)
{
   if(i == NULL)
      return;

   item_grid_remove(i);
   i->id = UINT64_MAX;

   *i->prev_next = i->next;
   if(i->next != NULL)
      i->next->prev_next = i->prev_next;

   i->next = item_pool;
   item_pool = i;
}

/*void item_remove(Item *i)
{
   if(i == NULL)
      return;

   item_grid_remove(i);

   i->id = UINT64_MAX;
   i->removed = 1;
}*/

void item_add(Area *a, Item *i)
{
   if(i==NULL)
      return;

   int cx = (i->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (i->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (i->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   int gx = (i->pos.x-(i->pos.x/32)*32) / 8;
   int gy = (i->pos.y-(i->pos.y/32)*32) / 8;
   int gz = (i->pos.z-(i->pos.z/32)*32) / 8;
   if(cx>0||cy<0||cz<0)
      return;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return;

   i->prev_next = &a->chunks[c]->items;
   if(a->chunks[c]->items!=NULL)
      a->chunks[c]->items->prev_next = &i->next;
   i->next = a->chunks[c]->items;
   a->chunks[c]->items = i;
}

void item_update_pos(Area *a, Item *i, Point new_pos)
{
   if(i==NULL)
      return;

   int cx = (i->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (i->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (i->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   int gx = (i->pos.x-(i->pos.x/32)*32) / 8;
   int gy = (i->pos.y-(i->pos.y/32)*32) / 8;
   int gz = (i->pos.z-(i->pos.z/32)*32) / 8;
   if(cx>0||cy<0||cz<0)
      return;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return;
   //if(new_pos.x<0||new_pos.y<0||new_pos.z<0)
      //return;
   //if(new_pos.x>=AREA_DIM * 32||new_pos.y>=AREA_DIM * 32||new_pos.z>=AREA_DIM * 32)
      //return;

   item_grid_remove(i);
   i->pos = new_pos;
   item_grid_add(a, i);
}

void item_grid_add(Area *a, Item *i)
{
   if(i==NULL)
      return;

   int cx = (i->pos.x/32)-a->cx+AREA_DIM/2;
   int cy = (i->pos.y/32)-a->cy+AREA_DIM/2;
   int cz = (i->pos.z/32)-a->cz+AREA_DIM/2;
   int c = cz*AREA_DIM*AREA_DIM+cy*AREA_DIM+cx;
   int gx = (i->pos.x-(i->pos.x/32)*32) / 8;
   int gy = (i->pos.y-(i->pos.y/32)*32) / 8;
   int gz = (i->pos.z-(i->pos.z/32)*32) / 8;
   int g = gz*4*4+gy*4+gx;
   if(cx>0||cy<0||cz<0)
      return;
   if(cx>=AREA_DIM||cy>=AREA_DIM||cz>=AREA_DIM)
      return;
   //if(i->pos.x<0||i->pos.y<0||i->pos.z<0)
      //return;
   //if(i->pos.x>=AREA_DIM * 32||i->pos.y>=AREA_DIM * 32||i->pos.z>=AREA_DIM * 32)
      //return;

   //int gx = i->pos.x / 8;
   //int gy = i->pos.y / 8;
   //int gz = i->pos.z / 8;
   //size_t g_index = gz * (AREA_DIM * 4) * (AREA_DIM * 4) + gy * (AREA_DIM * 4) + gx;

   i->g_prev_next = &a->chunks[c]->item_grid[g];
   if(a->chunks[c]->item_grid[g]!=NULL)
      a->chunks[c]->item_grid[g]->g_prev_next = &i->g_next;
   i->g_next = a->chunks[c]->item_grid[g];
   a->chunks[c]->item_grid[g] = i;
}

void item_grid_remove(Item *i)
{
   if(i==NULL)
      return;

   if(i->g_prev_next==NULL)
      return;

   *i->g_prev_next = i->g_next;
   if(i->g_next != NULL)
      i->g_next->g_prev_next = i->g_prev_next;
}

void item_sprite_create(Item *it)
{
   it->sprite = sprite_new(it);
   sprite_clear(it->sprite, 255);

   sprite_draw_sprite_remap(it->sprite, it->def->sprite, 0, 0, 0, 0, 32, 36, it->material.def->remap0, it->material.def->remap1, it->material.def->remap2, it->material.def->remap3);
}

void item_set_material(Item *it, const MaterialDef *def)
{
   it->material.def = def;
}

void item_from_def(Item *it, const ItemDef *def)
{
   it->def = def;
   if(def->tags & DEF_ITEM_SLOT_CONTAINER)
   {
      it->container.type = ITEM_SLOT_CONTAINER;
   }
}

//Not a deep copy!
Item *item_duplicate(World *w, const Item *i)
{
   Item *ni = item_new(w);
   ni->pos = i->pos;
   ni->sprite = i->sprite;
   ni->container = i->container;
   item_set_material(ni, i->material.def);
   item_from_def(ni, i->def);

   return ni;
}

const char *item_name(Item *it)
{
   if(it==NULL)
      return NULL;

   snprintf(item_name_buffer, 512, "%s %s", it->material.def->adjective, it->def->name);

   return item_name_buffer;
}
//-------------------------------------
