/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "region.h"
#include "area.h"
#include "area_draw.h"
#include "world.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define MEM_SIZE (1 << 27)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[MEM_SIZE];

static Area *area;
static World *world;
static Camera camera;
//-------------------------------------

//Function prototypes
static void loop();
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   RvR_malloc_init(mem, MEM_SIZE);

   RvR_init("RvnicRaven - iso", 0);

   RvR_pak_add("data/main.csv");

   //User defined overwrites (used for modding)
   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);

   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   world = world_new("test",WORLD_SMALL);
   area = area_create(world,0,0,1,1,1,0);
   camera.x = 16;
   camera.y = 0;
   camera.z = 1;

   while(RvR_running())
   {
      loop();
   }

   return 0;
}

static void loop()
{
   RvR_update();

   if(RvR_key_pressed(RVR_KEY_LEFT))
      camera.x--;
   if(RvR_key_pressed(RVR_KEY_RIGHT))
      camera.x++;
   if(RvR_key_pressed(RVR_KEY_UP))
      camera.y--;
   if(RvR_key_pressed(RVR_KEY_DOWN))
      camera.y++;

   if(RvR_key_pressed(RVR_KEY_Z))
      camera.z++;
   if(RvR_key_pressed(RVR_KEY_X))
      camera.z--;

   RvR_render_clear(0);
   area_draw(world,area,&camera);

   RvR_render_present();
}
//-------------------------------------
