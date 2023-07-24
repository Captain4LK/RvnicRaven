/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ITEM_H_

#define _ITEM_H_

#include "world_defs.h"
#include "item_defs.h"

Item *item_new(World *w);
void item_free(Item *i);

void item_remove(Item *i);
void item_add(Area *a, Item *i);

void item_update_pos(Area *a, Item *i, int16_t x, int16_t y, int16_t z);
void item_grid_add(Area *a, Item *i);
void item_grid_remove(Item *i);

#endif
