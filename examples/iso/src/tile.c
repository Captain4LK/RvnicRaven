/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
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

uint64_t tile_empty()
{
   return 0;
}

uint64_t tile_block(uint16_t id)
{
   uint64_t type = TILE_NONE;
   uint64_t bid = id&2047;
   return type|(bid<<3);
}

Tile_type tile_type(uint64_t tile)
{
   return (Tile_type)(tile&7);
}
//-------------------------------------
