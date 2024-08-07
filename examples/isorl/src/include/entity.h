/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ENTITY_H_

#define _ENTITY_H_

#include "world_defs.h"
#include "entity_defs.h"
#include "item_defs.h"
#include "defs.h"
#include "point.h"

Entity *entity_new(World *w);
void entity_free(Entity *e);

void entity_remove(Entity *e);
void entity_add(Area *a, Entity *e);

void entity_update_pos(Area *a, Entity *e, Point pos);
void entity_grid_add(Area *a, Entity *e);
void entity_grid_remove(Entity *e);

int entity_pos_valid(Area *a, Entity *e, Point pos);
unsigned entity_try_move(World *w, Area *a, Entity *e, uint8_t dir);
unsigned entity_try_ascend(Area *a, Entity *e);
unsigned entity_try_descend(Area *a, Entity *e);

void entity_think(World *w, Area *a, Entity *e);
void entity_turn(World *w, Area *a, Entity *e); //Runs everything that should happen once per turn (status effects, etc.)

int entity_move_cost(Entity *e);

void entity_from_def(Entity *e, const EntityDef *def, int female);

void entity_sprite_create(Entity *e);

//-1 --> random
void entity_hit(Entity *e, Entity *src, Item *weapon, int16_t body_part);

//Create corpse and remove entity
void entity_die(World *w, Area *a, Entity *e);

int entity_store_item(World *w, Area *a, Entity *e, Item *it);

int entity_can_equip(World *w, Area *a, Entity *e, Item *it, int check_space);
void entity_equip(World *w, Area *a, Entity *e, Item *it);

void entity_remove_item(World *w, Area *a, Entity *e, Item *it);
void entity_put_item(World *w, Area *a, Entity *e, Item *it, Item *container);

inline Entity_index entity_index_get(Entity *e) {
   if(e==NULL)
      return (Entity_index){
                .id = 0, .index = NULL
      }
   ; return (Entity_index){
             .id = e->id, .index = e
   };
}
inline Entity *entity_index_try(Entity_index index) { if(index.index==NULL) return NULL; if(index.index->id==index.id) return index.index; return NULL; }
inline int entity_list_length(const Entity *e) { int len = 0; for(const Entity *cur = e; cur!=NULL; cur = cur->next) len++; return len; }

#endif
