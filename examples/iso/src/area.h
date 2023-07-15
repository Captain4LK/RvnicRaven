/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _AREA_H_

#define _AREA_H_

#include "world_defs.h"

Area *area_create(World *w, uint16_t x, uint16_t y, uint8_t dimx, uint8_t dimy, uint8_t dimz, uint16_t id);
Area *area_load(World *w, uint16_t id);
uint32_t area_tile(const Area *a, int x, int y, int z);
void area_set_tile(Area *a, int x, int y, int z, uint32_t tile);

#endif
