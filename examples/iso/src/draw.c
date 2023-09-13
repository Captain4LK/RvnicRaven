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
      draw_end_x = width + (RvR_xres() - x - draw_end_x);
   if(y + draw_end_y>RvR_yres())
      draw_end_y = height + (RvR_yres() - y - draw_end_y);

   //Clip dst rect
   x = x<0?0:x;
   y = y<0?0:y;

   uint8_t *dst = &RvR_framebuffer()[x + y * RvR_xres()];
   int dst_step = RvR_xres() - (draw_end_x - draw_start_x);

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
            *dst = RvR_blend(col, *dst);
   }
   else
   {
      for(int y1 = draw_start_y; y1<draw_end_y; y1++, dst += dst_step)
         for(int x1 = draw_start_x; x1<draw_end_x; x1++, dst++)
            *dst = RvR_blend(*dst, col);
   }
}

void draw_string_wrap(int x, int y, int width, int height, int scale, const char *string, uint8_t col)
{
   RvR_texture *draw_font = RvR_texture_get(0xf000);

   int x_dim = draw_font->width / 16;
   int y_dim = draw_font->height / 6;
   int sx = 0;
   int sy = 0;
   width -= x_dim * scale;

   for(int i = 0; string[i]&&i<1024; i++)
   {
      if(string[i]==' ')
      {
         //Search next space (or newline)
         int len = 1;
         for(; string[i + len]!=' '&&string[i + len]!='\n'&&string[i + len]!='\0'; len++);
         if(sx + (len - 1) * x_dim * scale>=width)
         {
            sx = 0;
            sy += y_dim * scale;
            continue;
         }
      }

      if(string[i]=='\n'||sx>=width)
      {
         sx = 0;
         sy += y_dim * scale;
         continue;
      }

      int ox = (string[i] - 32) & 15;
      int oy = (string[i] - 32) / 16;
      for(int y_ = 0; y_<y_dim; y_++)
      {
         for(int x_ = 0; x_<x_dim; x_++)
         {
            if(!draw_font->data[(y_ + oy * y_dim) * draw_font->width + x_ + ox * x_dim])
               continue;
            for(int m = 0; m<scale; m++)
            {
               for(int o = 0; o<scale; o++)
               {
                  int dx = x + sx + (x_ * scale) + o;
                  int dy = y + sy + (y_ * scale) + m;
                  if(dx>=0&&dx<RvR_xres()&&dy>=0&&dy<RvR_yres())
                     RvR_framebuffer()[dy * RvR_xres() + dx] = col;
               }
            }
         }
      }
      sx += x_dim * scale;
   }
}
//-------------------------------------
