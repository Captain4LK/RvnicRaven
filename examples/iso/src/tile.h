/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _TILE_H_

#define _TILE_H_

typedef enum
{
   TILE_NONE = 0x0,
   TILE_WALL = 0x1,
   TILE_WALL_NW = 0x2,
   TILE_WALL_SW = 0x3,
   TILE_BLOCK = 0x4,
   TILE_SLOPE = 0x5,
   TILE_SSLOPEB = 0x6,
   TILE_SSLOPET = 0x7,
}Tile_type;

uint64_t tile_empty();
uint64_t tile_block(uint16_t id);

Tile_type tile_type(uint64_t tile);

#endif
