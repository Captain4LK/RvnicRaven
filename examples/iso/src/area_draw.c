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
#include "area_draw.h"
#include "tile.h"
#include "area.h"
#include "camera.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   uint16_t tex;
   uint16_t x;
   uint16_t y;
   uint16_t z;
}Sprite;
//-------------------------------------

//Variables
static const World *world;
static const Area *area;
static const Camera *cam;

static Sprite *sprites = NULL;
//-------------------------------------

//Function prototypes
static void draw_sprite(const RvR_texture *tex, int x, int y);
static void draw_sprite_bw(const RvR_texture *tex, int x, int y);

static int sprite_cmp(const void *a, const void *b);
static int pos_cmp(int x0, int y0, int z0, int x1, int y1, int z1);
//-------------------------------------

//Function implementations

void area_draw_begin(const World *w, const Area *a, const Camera *c)
{
   world = w;
   area = a;
   cam = c;

   RvR_array_length_set(sprites,0);
}

void area_draw_end()
{
   //Sort sprites
   qsort(sprites,RvR_array_length(sprites),sizeof(*sprites),sprite_cmp);
   int sprite_cur = 0;
   int sprite_max = RvR_array_length(sprites);

   int cx = cam->x*16+cam->y*16;
   int cy = cam->z*20-8*cam->x+8*cam->y;

   for(int z = area->dimz*32-1;z>=0;z--)
   {
      int origin_y = (16*cam->y-20*(z-cam->z))/16;
      int origin_x = -origin_y+cam->x+cam->y;
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
         max = RvR_min(area->dimx*32,max+2);

         for(int x = max;x>=min;x--)
         {
            //Sprites
            Sprite *sp = sprites+sprite_cur;
            if(sp->z==z&&sp->y==y&&sp->x==x)
            {
               draw_sprite(RvR_texture_get(sp->tex),x*16+y*16-cx,z*20-8*x+8*y-cy);
               sprite_cur++;
            }
            for(;sprite_cur<sprite_max&&pos_cmp(sprites[sprite_cur].x,sprites[sprite_cur].y,sprites[sprite_cur].z,x,y,z)<0;sprite_cur++);

            if(area_tile(area,x,y,z)==tile_set_discovered(0,1))
               continue;

            uint32_t tile = area_tile(area,x,y,z);
            uint32_t front = area_tile(area,x-1,y,z);
            uint32_t right = area_tile(area,x,y+1,z);
            uint32_t up = area_tile(area,x,y,z-1);

            if(tile_has_draw_slope(tile))
            {
               RvR_texture *tex = RvR_texture_get(tile_slope_texture(tile));
               draw_sprite(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);
               continue;
            }

            if(tile_has_draw_wall(tile)&&(!tile_has_draw_floor(tile)||!tile_has_draw_wall(front)||!tile_has_draw_wall(right)))
            {
               RvR_texture *tex = RvR_texture_get(tile_wall_texture(tile));
               draw_sprite(tex,x*16+y*16-cx,z*20-8*x+8*y-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }

            if(tile_has_draw_floor(tile)&&(!tile_has_draw_floor(front)||!tile_has_draw_floor(right)||!tile_has_draw_wall(up)))
            {
               RvR_texture *tex = RvR_texture_get(tile_floor_texture(tile));
               draw_sprite(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);

               if(RvR_key_pressed(RVR_KEY_SPACE))
                  RvR_render_present();
            }
         }
         y++;
      }
   }
}

void area_draw_sprite(uint16_t tex, int x, int y, int z)
{
   //Out of bounds
   if(x<0||y<0||z<0)
      return;
   if(x>=area->dimx*32||y>=area->dimy*32||z>=area->dimz*32)
      return;

   //Outside of screen
   RvR_texture *t = RvR_texture_get(tex);
   if(t==NULL)
      return;

   int cx = cam->x*16+cam->y*16;
   int cy = cam->z*20-8*cam->x+8*cam->y;
   int sx = x*16+y*16-cx;
   int sy = z*20-8*x+8*y-cy;

   //Left of screen
   if(sx+t->width/2<0)
      return;

   //Right of screen
   if(sx-t->width/2>=RvR_xres())
      return;

   //Below screen
   if(sy-t->height/2>=RvR_yres())
      return;

   //Above screen
   if(sy+t->height/2<0)
      return;

   //Push into list
   Sprite sp = {.tex = tex, .x = x, .y = y, .z = z};
   RvR_array_push(sprites,sp);
}

static void draw_sprite(const RvR_texture *tex, int x, int y)
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

static void draw_sprite_bw(const RvR_texture *tex, int x, int y)
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

static int sprite_cmp(const void *a, const void *b)
{
   const Sprite *sa = a;
   const Sprite *sb = b;

   if(sa->z==sb->z)
   {
      if(sa->y==sb->y)
         return (int)sb->x-(int)sa->x;

      return (int)sa->y-(int)sb->y;
   }

   return (int)sb->z-(int)sa->z;
}

static int pos_cmp(int x0, int y0, int z0, int x1, int y1, int z1)
{
   if(z0==z1)
   {
      if(y0==y1)
         return x1-x0;

      return y0-y1;
   }

   return z1-z0;
}
//-------------------------------------
