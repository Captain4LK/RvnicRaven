/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _BODY_H_

#define _BODY_H_

#include "defs.h"
#include "item_defs.h"

typedef struct
{
   const BodypartDef *def;
   int hp;
   int hp_max;
   int slot_count;
   Item_slot *slots;

   int16_t child;
   int16_t next;
}Bodypart;

typedef struct
{
   const BodyDef *def;
   uint16_t part_count;
   Bodypart *parts;
}Body;

void body_from_def(Body *body, const BodyDef *def);

#endif
