/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _SPIRAL_PATH_H_

#define _SPIRAL_PATH_H_

#include "world_defs.h"
#include "entity_defs.h"
#include "item_defs.h"
#include "point.h"

typedef enum
{
   FOV_OBJECT_ITEM,
   FOV_OBJECT_ENTITY,
}FOV_object_type;
typedef struct
{
   FOV_object_type type;
   union
   {
      Item *i;
      Entity *e;
   }as;
}FOV_object;

void fov_player(Area *a, Entity *e, Point old_pos);

//Valid until next fov_entity() call
FOV_object *fov_entity(Area *a, Entity *e);

#endif
