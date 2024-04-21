/*
RvnicRaven retro game engine

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define MEM_SIZE (1 << 24)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[MEM_SIZE];

static uint8_t color_black = 0;
static uint8_t color_white = 0;
//-------------------------------------

//Function prototypes
static uint8_t find_closest(RvR_color *pal, RvR_color color);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   if(argc<2)
   {
      puts("No pak path specified!");
      return -1;
   }

   RvR_malloc_init(mem, MEM_SIZE);

   RvR_init("RvnicRaven palette-viewer", 0);

   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);
   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   //Create colormap texture
   RvR_texture_create(65535,256,64);
   RvR_texture *tex = RvR_texture_get(65535);
   for(int y = 0;y<64;y++)
   {
      uint8_t *table = RvR_shade_table(y);
      for(int x = 0;x<256;x++)
         tex->data[y*tex->width+x] = table[x];
   }

   //Create transparancy texture
   RvR_texture_create(65534,256,256);
   tex = RvR_texture_get(65534);
   for(int y = 0;y<256;y++)
      for(int x = 0;x<256;x++)
         tex->data[y*tex->width+x] = RvR_blend(x,y);

   //Create palette texture

   //Find colors
   RvR_color *pal = RvR_palette();
   color_white = find_closest(pal, (RvR_color){.r = 255, .g = 255, .b = 255});
   color_black = find_closest(pal, (RvR_color){.r = 0, .g = 0, .b = 0});

   while(RvR_running())
   {
      RvR_update();

      RvR_render_clear(color_black);

      RvR_render_string(1,1,1,"Shade table",color_white);
      RvR_render_rectangle(1,9,260,68,color_white);
      tex = RvR_texture_get(65535);
      RvR_render_texture(tex,3,11);

      RvR_render_string(1,88,1,"Transparency table",color_white);
      RvR_render_rectangle(1,96,260,260,color_white);
      tex = RvR_texture_get(65534);
      RvR_render_texture(tex,3,98);

      RvR_render_present();
   }

   return 0;
}

static uint8_t find_closest(RvR_color *pal, RvR_color color)
{
   int dist_min = INT_MAX;
   uint8_t index_min = 0;

   for(int i = 0; i<256; i++)
   {
      int dr = pal[i].r - color.r;
      int dg = pal[i].g - color.g;
      int db = pal[i].b - color.b;
      int dist = dr * dr + dg * dg + db * db;
      if(dist<dist_min)
      {
         dist_min = dist;
         index_min = (uint8_t)i;
      }
   }

   return index_min;
}
//-------------------------------------
