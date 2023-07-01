/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "area_draw.h"
#include "tile.h"
#include "area.h"
#include "camera.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void area_draw_sprite(const RvR_texture *tex, int x, int y);
static void area_draw_sprite_bw(const RvR_texture *tex, int x, int y);
//-------------------------------------

//Function implementations

void area_draw(const World *w, const Area *a, const Camera *c)
{
   int cx = c->x*16+c->y*16;
   int cy = c->z*20-8*c->x+8*c->y;

   for(int z = a->dimz*32-1;z>=0;z--)
   {
      int origin_y = (16*c->y-20*(z-c->z))/16;
      int origin_x = -origin_y+c->x+c->y;
      int origin_z = z;
      int y = origin_y;

      //Would be infinite loop, but limited to prevent badness
      for(int i = 0;i<64;i++)
      {
         int min = RvR_max(-y+origin_x+origin_y,(20*(z-origin_z)+8*(y-origin_y)-RvR_yres())/8+origin_x);
         int max = RvR_min((RvR_xres()-16*(y-origin_y))/16+origin_x,(20*(z-origin_z)+8*(y-origin_y))/8+origin_x);

         if(min>max)
            break;

         min = RvR_max(0,min-1);
         max = RvR_min(a->dimx*32,max+2);

         for(int x = max;x>=min;x--)
         {
            if(area_tile(a,x,y,z)==tile_set_discovered(0,1))
               continue;

            uint32_t tile = area_tile(a,x,y,z);
            uint32_t front = area_tile(a,x-1,y,z);
            uint32_t right = area_tile(a,x,y+1,z);
            uint32_t up = area_tile(a,x,y,z-1);

            if(tile_has_draw_slope(tile))
            {
               RvR_texture *tex = RvR_texture_get(tile_slope_texture(tile));
               area_draw_sprite(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);
               continue;
            }

            if(tile_has_draw_wall(tile)&&(!tile_has_draw_floor(tile)||!tile_has_draw_wall(front)||!tile_has_draw_wall(right)))
            {
               RvR_texture *tex = RvR_texture_get(tile_wall_texture(tile));
               area_draw_sprite(tex,x*16+y*16-cx,z*20-8*x+8*y-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }

            if(tile_has_draw_floor(tile)&&(!tile_has_draw_floor(front)||!tile_has_draw_floor(right)||!tile_has_draw_wall(up)))
            {
               RvR_texture *tex = RvR_texture_get(tile_floor_texture(tile));
               area_draw_sprite(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }
         }
         y++;
      }
   }
}

static void area_draw_sprite(const RvR_texture *tex, int x, int y)
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
         *dst = *src?*src:*dst;
}

static void area_draw_sprite_bw(const RvR_texture *tex, int x, int y)
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
         *dst = *src?*src+128:*dst;
}
//-------------------------------------
