/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ENTITY_H_

#define _ENTITY_H_

#include "world_defs.h"
#include "entity_defs.h"

Entity *entity_new(World *w);
void entity_free(Entity *e);

void entity_remove(Entity *e);
void entity_add(Area *a, Entity *e);

void entity_update_pos(Area *a, Entity *e, int16_t x, int16_t y, int16_t z);
void entity_grid_add(Area *a, Entity *e);
void entity_grid_remove(Entity *e);

int entity_pos_valid(Area *a, Entity *e, int x, int y, int z);
unsigned entity_try_move(Area *a, Entity *e, int dir);
unsigned entity_try_ascend(Area *a, Entity *e);
unsigned entity_try_descend(Area *a, Entity *e);

#endif
