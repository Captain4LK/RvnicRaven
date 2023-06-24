/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _WORLD_DEFS_H_

#define _WORLD_DEFS_H_

#include "util.h"

typedef enum
{
   WORLD_SMALL = 0,
   WORLD_MEDIUM = 1,
   WORLD_LARGE = 2,
}World_size;

typedef struct
{
   //Dimensions in chunks (32x32x32)
   uint8_t dimx;
   uint8_t dimy;
   uint8_t dimz;

   uint16_t id;

   uint32_t *tiles;
}Area;

typedef struct
{
   uint16_t tiles[32*32];
}Region;

typedef struct
{
   char path[UTIL_PATH_MAX];
   int32_t *offset;
   int32_t offset_next;
}Region_file;

typedef struct
{
   char base_path[UTIL_PATH_MAX];

   World_size size;

   Region_file region_file;

   //Sparse array of regions --> cached
   Region **regions;
   uint16_t *region_map;
}World;

#endif
