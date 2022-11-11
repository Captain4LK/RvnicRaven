/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define CUTE_PATH_IMPLEMENTATION
#include "cute_path.h"

#define CUTE_FILES_IMPLEMENTATION
#include "cute_files.h"

#define RVR_MALLOC_IMPLEMENTATION
#define RVR_RW_IMPLEMENTATION
#define RVR_COMPRESS_IMPLEMENTATION
#define RVR_HASH_IMPLEMENTATION
#define RVR_PAK_IMPLEMENTATION
#define RVR_PALETTE_IMPLEMENTATION
#define RVR_TEXTURE_IMPLEMENTATION
#define RVR_MATH_IMPLEMENTATION
#define RVR_FIX22_IMPLEMENTATION
#define RVR_CORE_IMPLEMENTATION
#define RVR_DRAW_IMPLEMENTATION
#define RVR_RAY_IMPLEMENTATION
#include "RvR_malloc.h"
#include "RvR_rw.h"
#include "RvR_hash.h"
#include "RvR_compress.h"
#include "RvR_pak.h"
#include "RvR_palette.h"
#include "RvR_texture.h"
#include "RvR_math.h"
#include "RvR_fix22.h"
#include "RvR_core.h"
#include "RvR_draw.h"
#include "RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "color.h"
#include "texture.h"
#include "map.h"
#include "editor.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[1 << 25];
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   if(argc<2)
   {
      puts("No pak path specified!");
      return -1;
   }

   //Init memory manager
   RvR_malloc_init(mem, 1 << 25);

   //Init RvnicRaven core
   RvR_core_init("Rayed", 0);
   RvR_core_mouse_relative(0);
   RvR_core_mouse_show(0);
   RvR_core_key_repeat(1);

   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);
   RvR_palette_load(0);
   RvR_draw_font_set(0xF000);

   colors_find();
   texture_list_create();
   map_new(64, 64);
   texture_list_used_create();

   char path[512];
   path_pop(argv[0], path, NULL);
   strcat(path, "/");
   map_path_add(path);
   path_pop(argv[1], path, NULL);
   strcat(path, "/");
   map_path_add(path);

   editor_init();

   while(RvR_core_running())
   {
      RvR_core_update();

      editor_update();
      editor_draw();

      if(RvR_core_key_pressed(RVR_KEY_M))
         RvR_malloc_report();

      RvR_core_render_present();
   }

   map_set_path("autosave.map");
   map_save();

   return 0;
}
//-------------------------------------
