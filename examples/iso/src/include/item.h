/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ITEM_H_

#define _ITEM_H_

#include "world_defs.h"
#include "item_defs.h"
#include "defs.h"

Item *item_new(World *w);
void item_free(Item *i);

Item *item_duplicate(World *w, const Item *i);
//void item_remove(Item *i);
void item_add(Area *a, Item *i);

void item_update_pos(Area *a, Item *i, Point new_pos);
void item_grid_add(Area *a, Item *i);
void item_grid_remove(Item *i);

void item_sprite_create(Item *it);

void item_set_material(Item *it, const MaterialDef *def);
void item_from_def(Item *it, const ItemDef *def);

inline Item_index item_index_get(Item *it) { if(it==NULL) return (Item_index){.id = 0, .index = NULL}; return (Item_index){.id = it->id,.index = it}; }
inline Item *item_index_try(Item_index index) { if(index.index==NULL) return NULL; if(index.index->id==index.id) return index.index; return NULL; }

//returns temporary buffer
const char *item_name(Item *it);

#endif
