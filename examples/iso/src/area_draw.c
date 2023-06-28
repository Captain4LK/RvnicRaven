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
static void area_draw_wall(const RvR_texture *tex, int x, int y, uint8_t mode);
//-------------------------------------

//Function implementations

void area_draw(const World *w, const Area *a, const Camera *c)
{
   int cx = c->x*16+c->y*16;
   int cy = c->z*20-8*c->x+8*c->y;

   for(int z = a->dimz*32-1;z>=0;z--)
   {
      int origin_y = (16*c->y-20*(z-c->z)-24)/16;
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

         min = RvR_max(0,min-3);
         max = RvR_min(a->dimx*32,max-1);

         for(int x = max;x>=min;x--)
         {
            if(area_tile(a,x,y,z)==0)
               continue;

            uint32_t tile = area_tile(a,x,y,z);
            uint32_t front = area_tile(a,x-1,y,z);
            uint32_t right = area_tile(a,x,y+1,z);
            uint32_t up = area_tile(a,x,y,z-1);

            if(!tile_has_floor(tile)||!tile_has_wall(front)||!tile_has_wall(right))
            {
               RvR_texture *tex = RvR_texture_get(0);
               RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }

            if(!tile_has_floor(front)||!tile_has_floor(right)||!tile_has_wall(up))
            {
               RvR_texture *tex = RvR_texture_get(1);
               RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }
         }
         y++;
      }
   }
}

//Failed attempt at making drawing faster, basically
void area_draw_new(const World *w, const Area *a, const Camera *c)
{
   int cx = c->x*16+c->y*16;
   int cy = c->z*20-8*c->x+8*c->y;

   for(int z = a->dimz*32-1;z>=0;z--)
   {
      int origin_y = (16*c->y-20*(z-c->z)-24)/16;
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

         min = RvR_max(0,min-3);
         max = RvR_min(a->dimx*32,max-1);

         for(int x = max;x>=min;x--)
         {
            if(area_tile(a,x,y,z)==0)
               continue;

            uint32_t tile = area_tile(a,x,y,z);
            uint32_t front = area_tile(a,x-1,y,z);
            uint32_t right = area_tile(a,x,y+1,z);
            uint32_t up = area_tile(a,x,y,z-1);

            uint8_t mode_wall = (!tile_has_wall(front))|((!tile_has_wall(right))*2)|((!tile_has_floor(tile))*4);
            //if(!tile_has_floor(tile)||!tile_has_wall(front)||!tile_has_wall(right))
            {
               RvR_texture *tex = RvR_texture_get(0);
               area_draw_wall(tex,x*16+y*16-cx,z*20-8*x+8*y-cy,mode_wall);
               //RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }

            if(!tile_has_floor(front)||!tile_has_floor(right)||!tile_has_wall(up))
            {
               RvR_texture *tex = RvR_texture_get(1);
               RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }
         }
         y++;
      }
   }
}

static void area_draw_wall(const RvR_texture *tex, int x, int y, uint8_t mode)
{
   if(tex==NULL)
      return;

   if(mode==0||mode>7)
      return;

   const int offset_bounds[7][3] = {{0,24,8},{0,24,8},{0,24,8},{0,16,0}};
   const uint8_t offsets[7][64] = 
   {
      {0,1, 0,3, 0,5, 0,7, 0,9, 0,11, 0,13, 0,15, 0,16, 0,16, 0,16, 0,16, 0,16, 0,16, 0,16, 0,16, 1,16, 3,16, 5,16, 7,16, 9,16, 11,16, 13,16, 15,16},
      {31,32, 29,32, 27,32, 25,32, 23,32, 21,32, 19,32, 17,32, 16,32, 16,32, 16,32, 16,32, 16,32, 16,32, 16,32, 16,32, 16,31, 16,29, 16,27, 16,25, 16,23, 16,21, 16,19, 16,17 },
      { 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 0,32, 1,31, 3,29, 5,27, 7,25, 9,23, 11,21, 13,19, 15,17,},
      {},
   };

   y+=offset_bounds[mode-1][2];
   int offset_start = offset_bounds[mode-1][0];
   int offset_end = offset_bounds[mode-1][1];
   if(y<0)
      offset_start = -y;
   if(y+offset_end>RvR_yres())
      offset_end = offset_end+(RvR_yres()-y-offset_end);

   y = y<0?0:y;
   for(int i = offset_start;i<offset_end;i++,y++)
   {
      int start = offsets[mode-1][i*2];
      int end = offsets[mode-1][i*2+1];
      int dx = x+start;
      if(dx<0)
         start+=-dx;
      if(x+end>RvR_xres())
         end = end+(RvR_xres()-x-end);
      dx = dx<0?0:dx;

      const uint8_t *src = &tex->data[start + (i+offset_bounds[mode-1][2])* tex->width];
      uint8_t *dst = &RvR_framebuffer()[dx+y * RvR_xres()];
      for(int j = start;j<end;j++)
      {
         *dst = *src;
         src++;
         dst++;
      }

   }
}
//-------------------------------------
