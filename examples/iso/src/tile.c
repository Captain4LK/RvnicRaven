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

   return (tile&((1<<14)-1))>0;
}

int tile_has_floor(uint32_t tile)
{
   if(tile_is_slope(tile))
      return 0;

   return ((tile>>14)&((1<<14)-1))>0;
}

int tile_has_object(uint32_t tile)
{
   return !!(tile&(1<<29));
}

int tile_has_draw_wall(uint32_t tile)
{
   if(!tile_discovered(tile))
      return 1;

   return tile_has_wall(tile);
}

int tile_has_draw_floor(uint32_t tile)
{
   if(!tile_discovered(tile))
      return 1;

   return tile_has_floor(tile);
}

int tile_is_slope(uint32_t tile)
{
   return !!(tile&(1<<28));
}

int tile_visible(uint32_t tile)
{
   return !!(tile&(1<<31));
}

int tile_discovered(uint32_t tile)
{
   return !!(tile&(1<<30));
}

uint16_t tile_wall_texture(uint32_t tile)
{
   uint32_t wall_tile = (tile&((1<<14)-1));

   return wall_tile*2+1;
}

uint16_t tile_object_texture(uint32_t tile)
{
   uint32_t floor_tile = ((tile>>14)&((1<<14)-1));

   return 2+floor_tile+1;
}

uint16_t tile_floor_texture(uint32_t tile)
{
}

uint16_t tile_slope_texture(uint32_t tile)
{
}
//-------------------------------------
