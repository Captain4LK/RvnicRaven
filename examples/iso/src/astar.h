/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ASTAR_H_

#define _ASTAR_H_

#include "world_defs.h"
#include "point.h"

void astar_init(Area *a);
uint8_t *astar_path(Area *a, Entity *e, Point dst, uint32_t *path_len);

#endif
