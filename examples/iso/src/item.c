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
}

void item_add(Area *a, Item *i)
{
}

void item_update_pos(Area *a, Item *e, int16_t x, int16_t y, int16_t z)
{
}

void item_grid_add(Area *a, Item *e)
{
}

void item_grid_remove(Item *e)
{
}
//-------------------------------------
