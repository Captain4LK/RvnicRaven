/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _WORLD_DEFS_H_

#define _WORLD_DEFS_H_

#include "util.h"
#include "entity_defs.h"
#include "item_defs.h"

typedef enum
{
   WORLD_SMALL = 0,
   WORLD_MEDIUM = 1,
   WORLD_LARGE = 2,
}World_size;

typedef struct
{
   int32_t x;
   int32_t y;
   int32_t z;

   Entity *entities;
   Item *items;
   Entity *entity_grid[4*4*4];
   Item *item_grid[4*4*4];

   uint32_t tiles[32 * 32 * 32];
}Chunk;

//Currently loaded area
typedef struct
{
   //Position of center chunk in chunks
   int32_t cx;
   int32_t cy;
   int32_t cz;

   Chunk *chunks[AREA_DIM*AREA_DIM*AREA_DIM];

   //Entity *entities;
   //Item *items;

   //Entity *entity_grid[4*4*4*AREA_DIM*AREA_DIM*AREA_DIM];
   //Item *item_grid[4*4*4*AREA_DIM*AREA_DIM*AREA_DIM];
   //uint32_t tiles[32*32*32*AREA_DIM*AREA_DIM*AREA_DIM];
   //Dimensions in chunks (32x32x32)
   /*uint8_t dimx;
   uint8_t dimy;
   uint8_t dimz;

   uint16_t mx;
   uint16_t my;

   uint16_t id;

   uint32_t *tiles;

   Entity *entities;
   Entity **entity_grid;

   Item *items;
   Item **item_grid;*/
}Area;

typedef struct
{
   uint16_t tiles[32 * 32];
   uint16_t wetness[33 * 33];
   uint16_t temperature[33 * 33];

   //Sky -- ???
   struct
   {
      uint16_t top[33 * 33];
   }sky;

   //Surface
   struct
   {
      uint16_t top[33 * 33];
   }srf;

   //Surface
   //Edge points duplicated
   //-1 for uninitialized
   //int32_t elevation[33 * 33];
   //int32_t temperature[33 * 33];
   //int32_t rainfall[33 * 33];

   //Cavern 0 -- ???
   struct
   {
      uint16_t top[33 * 33];
      uint16_t bot[33 * 33];
   }cv0;

   //Cavern 1 -- ???
   struct
   {
      uint16_t top[33 * 33];
      uint16_t bot[33 * 33];
   }cv1;

   //Cavern 2 -- Hell
   struct
   {
      uint16_t top[33 * 33];
      uint16_t bot[33 * 33];
   }cv2;
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

   uint64_t next_eid; //entity id
   uint64_t next_deid; //docent id
   uint64_t next_iid; //item id
   uint64_t next_diid; //docit id

   World_size size;

   Region_file region_file;

   //Sparse array of regions --> cached
   Region **regions;
   uint16_t *region_map;

   //Preset
   struct
   {
      uint32_t lakes_deep;
      uint32_t lakes_shallow;
      uint32_t lakes_rand;

      uint32_t mountains_high;
      uint32_t mountains_medium;

      int32_t var_elevation;
      int32_t var_temperature;
      int32_t var_rainfall;
   }preset;

   //Documented entity hash table
   struct
   {
      Entity_documented **arr;
      uint8_t size_exp;
      int32_t count;
   }doctable;
}World;

#endif
