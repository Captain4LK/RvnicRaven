/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _AREA_GEN_H_

#define _AREA_GEN_H_

#include "world_defs.h"
#include "world_gen.h"

Area *area_gen(World *w, WorldGen_preset *p, uint32_t seed, int ax, int ay, int dimx, int dimy, int dimz, uint16_t id);

#endif
