/*
RvnicRaven retro game engine

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <string.h>
#include "tinyfiledialogs.h"
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "../config.h"
#include "../map.h"
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

int util_select_map_path()
{
   const char *filter_patterns[1] = {"*.map"};
   const char *file_path = tinyfd_saveFileDialog("Save map", NULL, 1, filter_patterns, NULL);

   if(file_path==NULL||strlen(file_path)==0)
      return 1;

   map_set_path(file_path);

   return 0;
}

int util_load_map()
{
   const char *filter_patterns[1] = {"*.map"};
   const char *file_path = tinyfd_openFileDialog("Select a map", NULL, 1, filter_patterns, NULL, 0);

   if(file_path==NULL||strlen(file_path)==0)
      return 1;

   map_load(file_path);

   return 0;
}
//-------------------------------------
