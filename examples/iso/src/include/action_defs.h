/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ACTION_DEFS_H_

#define _ACTION_DEFS_H_

#include "point.h"
#include "item_defs.h"

typedef enum
{
   ACTION_INVALID = -1,
   ACTION_WAIT = 0,
   ACTION_MOVE = 1,

   ACTION_ASCEND = 2,
   ACTION_DESCEND = 3,

   ACTION_ATTACK = 4,

   ACTION_PATH = 5,

   ACTION_PICKUP = 6,
   ACTION_DROP = 7,
   ACTION_EQUIP = 8,
   ACTION_REMOVE = 9,
   ACTION_PUT = 10,
}Action_id;

typedef enum
{
   ACTION_IN_PROGRESS = 0,

   ACTION_FINISHED = 1,
   ACTION_LEFT_MAP,
}Action_status;

typedef struct
{
   uint32_t time;
}AWait;

typedef struct
{
   uint8_t dir;
}AMove;

typedef struct
{
   uint8_t dir;
}AAttack;

typedef struct
{
   uint8_t *path;
   uint32_t pos;
   uint32_t len;
   Point goal;
   uint32_t flags;
}APath;

typedef struct
{
   Item_index item;
}APickup;

typedef struct
{
   Item_index item;
}ADrop;

typedef struct
{
   Item_index item;
}AEquip;

typedef struct
{
   Item_index item;
}ARemove;

typedef struct
{
   Item_index item;
   Item_index container;
}APut;

typedef struct
{
   Action_id id;
   int status;

   int cost;
   int interrupt;
   int can_interrupt;

   union
   {
      AWait wait;
      AMove move;
      AAttack attack;
      APath path;
      APickup pickup;
      ADrop drop;
      AEquip equip;
      ARemove remove;
      APut put;
   }as;
}Action;

#endif
