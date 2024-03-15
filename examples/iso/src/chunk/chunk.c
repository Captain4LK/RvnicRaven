/*
RvnicRaven - iso roguelike

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "world_gen.h"
#include "world.h"
#include "area.h"
#include "tile.h"
#include "region.h"
#include "chunk.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static Chunk *chunk_gen(uint16_t x, uint16_t y, uint16_t z);
//-------------------------------------

//Function implementations

Chunk *chunk_get(uint16_t x, uint16_t y, uint16_t z)
{
   //TODO(Captian4LK): check if chunk in existing area

   return chunk_gen(x,y,z);
}

static Chunk *chunk_gen(uint16_t x, uint16_t y, uint16_t z)
{
   return NULL;
}
//-------------------------------------
