/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "state.h"
#include "draw.h"
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
static void loop();
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   RvR_malloc_init(mem, MEM_SIZE);

   RvR_init("RvnicRaven - iso", 0);
   RvR_key_repeat(1);

   RvR_pak_add("data/main.csv");

   //User defined overwrites (used for modding)
   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);

   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   while(RvR_running())
   {
      loop();
   }

   return 0;
}

static void loop()
{
   RvR_update();

   draw_begin();
   draw_map();
   draw_end();
   /*
   GRP_floor *grp = grp_floor_get(0);
   for(int i = 0;i<32;i++)
   {
      uint32_t pos = grp->row_offsets[i];
      int count = grp->data[pos++];
      for(int j = 0;j<count;j++)
      {
         int start = grp->data[pos++];
         int len = grp->data[pos++];
         for(int l = 0;l<len;l++)
         {
            RvR_framebuffer()[(i+16)*RvR_xres()+start+l+16] = grp->data[pos++];
         }
      }
      //if(grp->row_offsets[i]==UINT32_MAX)
         //continue;
   }
   //printf("%d %d\n",grp->data[0],grp->data[1]);
   */

   RvR_render_present();
}
//-------------------------------------

