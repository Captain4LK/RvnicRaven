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
#include "world_gen.h"
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

void world_gen(World *w, uint32_t seed)
{
   RvR_rand_pcg rand = {0};
   RvR_rand_pcg_seed(&rand,seed);

   unsigned dim = world_size_to_dim(w->size);
   int32_t *elevation = RvR_malloc((dim+1)*32*(dim+1)*32,"Map Gen elevation");
   for(int i = 0;i<(dim+1)*32*(dim+1)*32;i++)
   {
      elevation[i] = -1;
   }

   //Basic parameters
   //-------------------------------------

   //Init with random elevation above sea level
   //TODO(Captain4LK): weigh with circle
   for(int i = 0;i<(dim+1)*(dim+1);i++)
   {
      int x = i%(dim+1);
      int y = i/(dim+1);

      //Sea level at 128*1024
      elevation[y*(dim+1)*32+x*32] = RvR_rand_pcg_next_range(&rand,124*1024,196*1024);
   }
  
   //Corners are below sea level
   for(int i = 0;i<dim+1;i++)
   {
      elevation[i*32] = 0;
      elevation[dim*32*dim*32+i*32] = 64*1024;
   }

   //Place up to n mountainpeaks

   //-------------------------------------

   //Diamond-Square for filling in
   //-------------------------------------



   //-------------------------------------

   //Save to regions

   RvR_free(elevation);
}
//-------------------------------------
