/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _WORLD_GEN_H_

#define _WORLD_GEN_H_

#include "world_defs.h"

typedef struct
{
   unsigned lakes_deep;
   unsigned lakes_shallow;
   unsigned lakes_rand;

   unsigned mountains_high;
   unsigned mountains_medium;

   int32_t var_elevation;
   int32_t var_temperature;
   int32_t var_rainfall;
}WorldGen_preset;

void world_gen(World *w, uint32_t seed, WorldGen_preset *preset);

#endif
