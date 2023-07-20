/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "tile.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int tile_has_wall(uint32_t tile)
{
   if(tile_is_slope(tile))
      return 0;

   if(tile_has_object(tile))
      return 0;

   return (tile & ((1 << 14) - 1))>0;
}

int tile_has_floor(uint32_t tile)
{
   if(tile_is_slope(tile))
      return 0;

   return ((tile >> 14) & ((1 << 12) - 1))>0;
}

int tile_has_object(uint32_t tile)
{
   return !!(tile & (1 << 29));
}

int tile_has_draw_wall(uint32_t tile)
{
   if(!tile_discovered_wall(tile))
      return 1;

   return tile_has_wall(tile);
}

int tile_has_draw_floor(uint32_t tile)
{
   if(!tile_discovered_floor(tile))
      return 1;

   return tile_has_floor(tile);
}

int tile_has_draw_slope(uint32_t tile)
{
   if(!tile_discovered_wall(tile))
      return 0;

   return tile_is_slope(tile);
}

int tile_is_slope(uint32_t tile)
{
   return !!(tile & (1 << 28));
}

int tile_is_usable_slope(uint32_t tile)
{
   if(!tile_is_slope(tile))
      return 0;

   uint32_t variant = ((tile >> 14) & ((1 << 12) - 1));

   return variant!=12;
}

int tile_visible_wall(uint32_t tile)
{
   return !!(tile & (1 << 31));
}

int tile_discovered_wall(uint32_t tile)
{
   return !!(tile & (1 << 30));
}

int tile_visible_floor(uint32_t tile)
{
   return !!(tile & (1 << 27));
}

int tile_discovered_floor(uint32_t tile)
{
   return !!(tile & (1 << 26));
}

uint32_t tile_set_visible(uint32_t tile, int vis_wall, int vis_floor)
{
   return tile_set_visible_wall(tile_set_visible_floor(tile, vis_floor), vis_wall);
}

uint32_t tile_set_discovered(uint32_t tile, int disc_wall, int disc_floor)
{
   return tile_set_discovered_wall(tile_set_discovered_floor(tile, disc_floor), disc_wall);
}

uint32_t tile_set_visible_wall(uint32_t tile, int visible)
{
   uint32_t value = (!!visible) << 31;
   uint32_t bit = 1 << 31;

   return (tile & (~bit)) | value;
}

uint32_t tile_set_discovered_wall(uint32_t tile, int discovered)
{
   uint32_t value = (!!discovered) << 30;
   uint32_t bit = 1 << 30;

   return (tile & (~bit)) | value;
}

uint32_t tile_set_visible_floor(uint32_t tile, int visible)
{
   uint32_t value = (!!visible) << 27;
   uint32_t bit = 1 << 27;

   return (tile & (~bit)) | value;
}

uint32_t tile_set_discovered_floor(uint32_t tile, int discovered)
{
   uint32_t value = (!!discovered) << 26;
   uint32_t bit = 1 << 26;

   return (tile & (~bit)) | value;
}

uint32_t tile_make_wall(uint16_t wall, uint16_t floor)
{
   wall = wall & ((1 << 14) - 1);
   floor = floor & ((1 << 12) - 1);

   return wall | (floor << 14);
}

uint32_t tile_make_object(uint16_t object, uint16_t floor)
{
   object = object & ((1 << 14) - 1);
   floor = floor & ((1 << 12) - 1);

   return object | (floor << 14) | (1 << 29);
}

uint32_t tile_make_slope(uint16_t slope, uint16_t variant)
{
   slope = slope & ((1 << 14) - 1);
   variant = variant & ((1 << 12) - 1);

   return slope | (variant << 14) | (1 << 28);
}

uint16_t tile_wall_texture(uint32_t tile)
{
   if(!tile_discovered_wall(tile))
      return 0;
   uint32_t wall_tile = (tile & ((1 << 14) - 1));

   return 2 + (wall_tile - 1) * 16;
}

uint16_t tile_object_texture(uint32_t tile)
{
   return 0;
}

uint16_t tile_floor_texture(uint32_t tile)
{
   if(!tile_discovered_floor(tile))
      return 1;
   uint32_t floor_tile = ((tile >> 14) & ((1 << 12) - 1));

   return 2 + (floor_tile - 1) * 16 + 1;
}

uint16_t tile_slope_texture(uint32_t tile, uint8_t rotation)
{
   uint32_t slope = (tile & ((1 << 14) - 1));
   uint32_t variant = ((tile >> 14) & ((1 << 12) - 1));

   if(variant<4) variant = (variant + rotation) & 3;
   else if(variant<8) variant = 4 + ((variant - 4 + rotation) & 3);
   else if(variant<12) variant = 8 + ((variant - 8 + rotation) & 3);

   return 2 + (slope - 1) * 16 + variant + 2;
}
//-------------------------------------
