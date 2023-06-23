/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _REGION_H_

#define _REGION_H_

#include "world_defs.h"

//Regions are tagged RVR_MALLOC_CACHE, with the world regions array as usr
Region *region_create(World *w, unsigned x, unsigned y);
Region *region_load(World *w, unsigned x, unsigned y, int create_if_missing);

#endif
