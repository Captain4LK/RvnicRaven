/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "draw.h"
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

void draw_fill_rectangle(int x, int y, int width, int height, uint8_t col, int transparent)
{
   //Clip src rect
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = width;
   int draw_end_y = height;

   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x + draw_end_x>RvR_xres())
      draw_end_x = width + (RvR_xres()- x - draw_end_x);
   if(y + draw_end_y>RvR_yres())
      draw_end_y = height + (RvR_yres()- y - draw_end_y);

   //Clip dst rect
   x = x<0?0:x;
   y = y<0?0:y;

   uint8_t *dst = &RvR_framebuffer()[x + y * RvR_xres()];
   int dst_step = RvR_xres()- (draw_end_x - draw_start_x);

   if(transparent==0)
   {
      for(int y1 = draw_start_y; y1<draw_end_y; y1++, dst += dst_step)
         for(int x1 = draw_start_x; x1<draw_end_x; x1++, dst++)
            *dst = col;
   }
   else if(transparent==1)
   {
      for(int y1 = draw_start_y; y1<draw_end_y; y1++, dst += dst_step)
         for(int x1 = draw_start_x; x1<draw_end_x; x1++, dst++)
            *dst = RvR_blend(col,*dst);
   }
   else
   {
      for(int y1 = draw_start_y; y1<draw_end_y; y1++, dst += dst_step)
         for(int x1 = draw_start_x; x1<draw_end_x; x1++, dst++)
            *dst = RvR_blend(*dst,col);
   }
}
//-------------------------------------
