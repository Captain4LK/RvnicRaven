/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "map.h"
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

Map *map_create(int dim_x, int dim_y, int dim_z)
{
   Map *map = RvR_malloc(sizeof(*map) + sizeof(*map->tiles) * dim_x * dim_y * dim_z, "iso map");
   map->dim_x = dim_x;
   map->dim_y = dim_y;
   map->dim_z = dim_z;

   srand(0);
   for(int z = 0; z<dim_z; z++)
   {
      for(int y = 0; y<dim_y; y++)
      {
         for(int x = 0; x<dim_x; x++)
         {
            if(rand() & 1)
               map->tiles[z * dim_y * dim_x + y * dim_x + x] = tile_empty();
            else
               map->tiles[z * dim_y * dim_x + y * dim_x + x] = tile_block(0);
         }
      }
   }

   return map;
}
//-------------------------------------
