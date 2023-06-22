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

World *world_new(const char *name)
{
   if(name==NULL)
      return NULL;

   World *w = RvR_malloc(sizeof(*w),"World struct");
   memset(w,0,sizeof(*w));

   util_mkdir("saves");
   snprintf(w->base_path,UTIL_PATH_MAX,"./saves/%s",name);
   util_mkdir(w->base_path);

   return w;
}
//-------------------------------------
