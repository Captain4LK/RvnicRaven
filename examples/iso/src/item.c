/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "item.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Item *item_pool = NULL;
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

   uint32_t id = w->next_iid++;
   memset(n, 0, sizeof(*n));
   n->next = NULL;
   n->prev_next = NULL;
   n->g_next = NULL;
   n->g_prev_next = NULL;
   n->id = id;

   return n;
}

void item_free(Item *i)
{
   if(i == NULL)
      return;

   *i->prev_next = i->next;
   if(i->next != NULL)
      i->next->prev_next = i->prev_next;

   i->next = item_pool;
   item_pool = i;
}

void item_remove(Item *i)
{
   if(i == NULL)
      return;

   item_grid_remove(i);

   i->id = UINT64_MAX;
   i->removed = 1;
}

void item_add(Area *a, Item *i)
{
   if(i==NULL)
      return;

   i->prev_next = &a->items;
   if(a->items!=NULL)
      a->items->prev_next = &i->next;
   i->next = a->items;
   a->items = i;
}

void item_update_pos(Area *a, Item *i, int16_t x, int16_t y, int16_t z)
{
   if(i==NULL)
      return;
   if(x<0||y<0||z<0)
      return;
   if(x>=a->dimx * 32||y>=a->dimy * 32||z>=a->dimz * 32)
      return;

   item_grid_remove(i);
   i->x = x;
   i->y = y;
   i->z = z;
   item_grid_add(a, i);
}

void item_grid_add(Area *a, Item *i)
{
   if(i==NULL)
      return;
   if(i->x<0||i->y<0||i->z<0)
      return;
   if(i->x>=a->dimx * 32||i->y>=a->dimy * 32||i->z>=a->dimz * 32)
      return;

   int gx = i->x / 8;
   int gy = i->y / 8;
   int gz = i->z / 8;
   size_t g_index = gz * (a->dimx * 4) * (a->dimy * 4) + gy * (a->dimx * 4) + gx;
   i->g_prev_next = &a->item_grid[g_index];
   if(a->item_grid[g_index]!=NULL)
      a->item_grid[g_index]->prev_next = &i->g_next;
   i->g_next = a->item_grid[g_index];
   a->item_grid[g_index] = i;
}

void item_grid_remove(Item *i)
{
   *i->g_prev_next = i->g_next;
   if(i->g_next != NULL)
      i->g_next->prev_next = i->g_prev_next;
}
//-------------------------------------
