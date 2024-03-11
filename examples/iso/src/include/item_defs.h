/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ITEM_DEFS_H_

#define _ITEM_DEFS_H_

#include "defs.h"
#include "point.h"
#include "material_defs.h"

typedef struct Item Item;

struct Item
{
   Point pos;
   uint16_t sprite;

   uint64_t id;

   int removed;

   Material material;
   const ItemDef *def;

   Item *next;
   Item **prev_next;

   Item *g_next;
   Item **g_prev_next;
};

typedef enum
{
   ITEM_SLOT_UPPER,
   ITEM_SLOT_LOWER,
   ITEM_SLOT_HEAD,
   ITEM_SLOT_HAND,
   ITEM_SLOT_FOOT,
   ITEM_SLOT_GRASP,
}Item_slot_type;

typedef enum
{
   ITEM_SLOT_ANY = 0,
   ITEM_SLOT_UNDER,
   ITEM_SLOT_OVER,
   ITEM_SLOT_ARMOR,
   ITEM_SLOT_BACK,
}Item_slot_layer;

typedef struct
{
   Item_slot_type type;
   Item_slot_layer layer;
   Item *it;
}Item_slot;

typedef struct
{
   uint64_t id;
   Item *index;
}Item_index;

#endif
