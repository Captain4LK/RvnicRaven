/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "color.h"
#include "texture.h"
#include "map.h"
#include "editor.h"

#include "../../libraries/HLH_gui/HLH_gui.h"
#include "../../libraries/HLH_gui/HLH_gui_all.c"
#include "RvR/RvR_gui.h"
//-------------------------------------

//#defines
#define MEM_SIZE (1 << 24)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[MEM_SIZE];
//-------------------------------------

//Function prototypes
static void main_loop();
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   HLH_gui_init();

   HLH_gui_window *win = HLH_gui_window_create("Ported",800,600,NULL);
   HLH_gui_group *group = HLH_gui_group_create(&win->e,HLH_GUI_EXPAND);

   if(argc<2)
   {
      puts("No pak path specified!");
      return -1;
   }

   //Init memory manager
   RvR_malloc_init(mem, MEM_SIZE);

   //Init RvnicRaven core
   HLH_gui_rvr *rvr = HLH_gui_rvr_create(&group->e,HLH_GUI_EXPAND,main_loop);
   RvR_init("Ported", 0);
   RvR_mouse_relative(0);
   RvR_mouse_show(0);
   RvR_key_repeat(1);

   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);
   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   colors_find();
   texture_list_create();
   map_new();
   texture_list_used_create();

   char path[512];
   path_pop(argv[0], path, NULL);
   strcat(path, "/");
   map_path_add(path);
   path_pop(argv[1], path, NULL);
   strcat(path, "/");
   map_path_add(path);

   editor_init();

   /*while(RvR_running())
   {
      RvR_update();

      editor_update();
      editor_draw();

      if(RvR_key_pressed(RVR_KEY_M))
         RvR_malloc_report();

      RvR_render_present();
   }

   map_set_path("autosave.map");
   map_save();*/

   return HLH_gui_message_loop();
}

static void main_loop()
{
   RvR_update();

   editor_update();
   editor_draw();

   if(RvR_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   RvR_render_present();
}
//-------------------------------------
