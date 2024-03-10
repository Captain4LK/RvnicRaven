/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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

void draw_line_vertical(int x, int y0, int y1, uint8_t col, int transparent)
{
   if(y0>y1)
   {
      int t = y0;
      y0 = y1;
      y1 = t;
   }

   if(x<0||x>=RvR_xres()||y0>=RvR_yres()||y1<0)
      return;

   if(y0<0)
      y0 = 0;
   if(y1>=RvR_yres())
      y1 = RvR_yres() - 1;

   uint8_t *buff = RvR_framebuffer();

   if(transparent==0)
   {
      for(int y = y0; y<=y1; y++)
         buff[y * RvR_xres() + x] = col;
   }
   else if(transparent==1)
   {
      for(int y = y0; y<=y1; y++)
         buff[y * RvR_xres() + x] = RvR_blend(col, buff[y * RvR_xres() + x]);
   }
   else
   {
      for(int y = y0; y<=y1; y++)
         buff[y * RvR_xres() + x] = RvR_blend(buff[y * RvR_xres() + x], col);
   }
}

void draw_line_horizontal(int x0, int x1, int y, uint8_t col, int transparent)
{
   if(x0>x1)
   {
      int t = x0;
      x0 = x1;
      x1 = t;
   }

   if(y<0||y>=RvR_yres()||x0>=RvR_xres()||x1<0)
      return;

   if(x0<0)
      x0 = 0;
   if(x1>=RvR_xres())
      x1 = RvR_xres() - 1;

   uint8_t *dst = &RvR_framebuffer()[y * RvR_xres() + x0];

   if(transparent==0)
   {
      for(int x = x0; x<=x1; x++, dst++)
         *dst = col;
   }
   else if(transparent==1)
   {
      for(int x = x0; x<=x1; x++, dst++)
         *dst = RvR_blend(col, *dst);
   }
   else
   {
      for(int x = x0; x<=x1; x++, dst++)
         *dst = RvR_blend(*dst, col);
   }
}

void draw_sprite(const RvR_texture *tex, int x, int y)
{
   if(tex==NULL)
      return;

   //Clip source texture
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = tex->width;
   int draw_end_y = tex->height;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x + draw_end_x>640)
      draw_end_x = tex->width + (640 - x - draw_end_x);
   if(y + draw_end_y>480)
      draw_end_y = tex->height + (480 - y - draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t * restrict src = &tex->data[draw_start_x + draw_start_y * tex->width];
   uint8_t * restrict dst = &RvR_framebuffer()[x + y * 640];
   int src_step = -(draw_end_x - draw_start_x) + tex->width;
   int dst_step = 640 - (draw_end_x - draw_start_x);

   for(int y1 = draw_start_y; y1<draw_end_y; y1++, dst += dst_step, src += src_step)
      for(int x1 = draw_start_x; x1<draw_end_x; x1++, src++, dst++)
         *dst = (*src!=255)?*src:*dst;
}

void draw_sprite_bw(const RvR_texture *tex, int x, int y)
{
   if(tex==NULL)
      return;

   //Clip source texture
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = tex->width;
   int draw_end_y = tex->height;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x + draw_end_x>640)
      draw_end_x = tex->width + (640 - x - draw_end_x);
   if(y + draw_end_y>480)
      draw_end_y = tex->height + (480 - y - draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t * restrict src = &tex->data[draw_start_x + draw_start_y * tex->width];
   uint8_t * restrict dst = &RvR_framebuffer()[x + y * 640];
   int src_step = -(draw_end_x - draw_start_x) + tex->width;
   int dst_step = 640 - (draw_end_x - draw_start_x);

   for(int y1 = draw_start_y; y1<draw_end_y; y1++, dst += dst_step, src += src_step)
      for(int x1 = draw_start_x; x1<draw_end_x; x1++, src++, dst++)
         *dst = (*src!=255)?*src + 128:*dst;
}
//-------------------------------------
