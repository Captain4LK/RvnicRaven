/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _WORLD_H_

#define _WORLD_H_

#include "world_defs.h"

void world_load(const char *base_path);
World *world_new(const char *name, World_size size);
void world_free(World *w);

unsigned world_size_to_dim(World_size size);

#endif
