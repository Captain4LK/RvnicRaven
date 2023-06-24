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
#include "util.h"
#include "world.h"
#include "region.h"
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

World *world_new(const char *name, World_size size)
{
   if(name==NULL)
      return NULL;

   World *w = RvR_malloc(sizeof(*w),"World struct");
   memset(w,0,sizeof(*w));

   util_mkdir("saves");
   snprintf(w->base_path,UTIL_PATH_MAX,"./saves/%s",name);
   util_mkdir(w->base_path);

   region_file_create(w);

   w->size = size;
   int dim = world_size_to_dim(size);
   w->regions = RvR_malloc(sizeof(*w->regions)*dim*dim,"World regions");
   w->region_map = RvR_malloc(sizeof(*w->region_map)*dim*dim,"World region map");

   return w;
}

void world_free(World *w)
{
   if(w==NULL)
      return;

   RvR_free(w->regions);
   RvR_free(w->region_map);
   RvR_free(w);
}

unsigned world_size_to_dim(World_size size)
{
   switch(size)
   {
   case WORLD_SMALL:
      return 16;
   case WORLD_MEDIUM:
      return 32;
   case WORLD_LARGE:
      return 64;
   }

   return 0;
}
//-------------------------------------
