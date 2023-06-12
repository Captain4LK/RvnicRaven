/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "sprite.h"
#include "state.h"
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
static void loop();
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   RvR_malloc_init(mem, MEM_SIZE);

   RvR_init("RvnicRaven - stargazer", 0);
   RvR_mouse_relative(1);

   RvR_pak_add("data/main.csv");

   //User defined overwrites (used for modding)
   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);

   config_read("settings.ini");

   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   sprites_init();

   //Second framebuffer for effects
   RvR_texture_create(65535, RvR_xres(), RvR_yres());

   state_init(STATE_GAME_INVENTORY);
   state_init(STATE_TITLE);

   while(RvR_running())
   {
      loop();
   }

   config_write("settings.ini");
}

static void loop()
{
   RvR_update();

   state_update();
   state_draw();

   RvR_render_present();
}
//-------------------------------------
