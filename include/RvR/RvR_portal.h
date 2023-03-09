/*
RvnicRaven - Portal renderer

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PORTAL_H_

#define _RVR_PORTAL_H_

typedef struct
{
   int16_t wall_count;
   int16_t wall_first;
}RvR_port_sector;

typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   uint32_t flags;
   int16_t p2;
   int16_t portal;
}RvR_port_wall;

typedef struct
{
   RvR_port_wall *walls;
   RvR_port_sector *sectors;
   int32_t wall_count;
   int32_t sector_count;
}RvR_port_map;

int RvR_port_sector_inside(RvR_port_map *map, int16_t sector, RvR_fix16 x, RvR_fix16 y);

#endif