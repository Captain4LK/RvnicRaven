/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _MAP_H_

#define _MAP_H_

typedef struct
{
   int dim_x;
   int dim_y;
   int dim_z;
   uint64_t tiles[];
}Map;

Map *map_create(int dim_x, int dim_y, int dim_z);

#endif
