/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _AREA_H_

#define _AREA_H_

#include "world_defs.h"
#include "point.h"

Area *area_create(World *w, int16_t x, int16_t y, int16_t z);
uint32_t area_tile(const Area *a, Point pos);
void area_set_tile(Area *a, Point pos, uint32_t tile);
Entity *area_entity_at(Area * a, Point pos, Entity * not);

//Area *area_create(World *w, uint16_t x, uint16_t y, uint8_t dimx, uint8_t dimy, uint8_t dimz, uint16_t id);
//void area_free(World *w, Area *a);
//void area_exit(World *w, Area *a);
//Area *area_load(World *w, uint16_t id);
//void area_save(World *w, Area *a);

//uint32_t area_tile(const Area *a, Point pos);
//void area_set_tile(Area *a, Point pos, uint32_t tile);

//Entity *area_entity_at(Area * a, Point pos, Entity * not);

#endif
