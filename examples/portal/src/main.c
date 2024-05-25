/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "state.h"
//-------------------------------------

//#defines
#define MEM_SIZE (1 << 27)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[MEM_SIZE];
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   RvR_malloc_init(mem, MEM_SIZE);

   RvR_init("RvnicRaven - portal", 0);
   RvR_key_repeat(1);
   RvR_mouse_relative(1);

   config_read("settings.ini");

   RvR_pak_add("data/main.csv");

   //User defined overwrites (used for modding)
   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);

   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   state_init(STATE_GAME);
   state_set(STATE_GAME);

   while(RvR_running())
   {
      RvR_update();

      state_update();
      state_draw();

      if(RvR_key_pressed(RVR_KEY_M))
         RvR_malloc_report();

      RvR_render_present();
   }

   config_write("settings.ini");

   return 0;
}
//-------------------------------------
