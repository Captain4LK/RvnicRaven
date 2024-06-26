/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "color.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
uint8_t color_white = 0;
uint8_t color_light_gray = 0;
uint8_t color_dark_gray = 0;
uint8_t color_black = 0;
uint8_t color_magenta = 0;
uint8_t color_yellow = 0;
uint8_t color_red = 0;
uint8_t color_orange = 0;
uint8_t color_aqua = 0;
//-------------------------------------

//Function prototypes
static uint8_t find_closest(RvR_color *pal, RvR_color color);
//-------------------------------------

//Function implementations

void colors_find()
{
   RvR_color *pal = RvR_palette();

   color_white = find_closest(pal, (RvR_color){.r = 255, .g = 255, .b = 255});
   color_dark_gray = find_closest(pal, (RvR_color){.r = 40, .g = 40, .b = 40});
   color_light_gray = find_closest(pal, (RvR_color){.r = 180, .g = 180, .b = 180});
   color_black = find_closest(pal, (RvR_color){.r = 0, .g = 0, .b = 0});
   color_magenta = find_closest(pal, (RvR_color){.r = 255, .g = 0, .b = 255});
   color_yellow = find_closest(pal, (RvR_color){.r = 255, .g = 255, .b = 100});
   color_red = find_closest(pal, (RvR_color){.r = 255, .g = 0, .b = 0});
   color_orange = find_closest(pal, (RvR_color){.r = 255, .g = 128, .b = 0});
   color_aqua = find_closest(pal, (RvR_color){.r = 0, .g = 255, .b = 255});
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
