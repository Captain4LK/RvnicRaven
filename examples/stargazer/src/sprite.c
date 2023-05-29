/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 depth;
   uint16_t texture;
   uint16_t flags;
}Sprite_draw;
//-------------------------------------

//Variables
struct
{
   Sprite_draw * restrict data;
   uint_fast16_t * restrict data_proxy;
   uint32_t data_used;
   uint32_t data_size;
}sprite_stack = {0};

static RvR_fix16 view_fov_factor_x;
static RvR_fix16 view_fov_factor_y;
static RvR_fix16 view_sin;
static RvR_fix16 view_cos;
static RvR_fix16 view_sin_fov;
static RvR_fix16 view_cos_fov;
static RvR_fix16 view_middle_row;

Sprite sprites[1<<16];
//-------------------------------------

//Function prototypes
static void sprite_stack_push(const Sprite_draw *s);

static int sprite_cmp(const void *a, const void *b);
//-------------------------------------

//Function implementations

void sprites_init(void)
{
   //Default settings:
   //   * no rotation
   //   * texture and index are the same
   //   * no flags
   for(int i = 0;i<1<<16;i++)
   {
      sprites[i].rot = 0;
      sprites[i].tex[0] = i;
      sprites[i].flag[0] = 0;
   }
}

void sprite_draw_begin()
{
   sprite_stack.data_used = 0;

   /*view_fov_factor_x = RvR_fix16_tan(RvR_ray_get_fov()/2);
   view_fov_factor_y = (RVR_YRES*view_fov_factor_x*2)/RVR_XRES;
   view_cos = RvR_fix22_cos(RvR_ray_get_angle());
   view_sin = RvR_fix22_sin(RvR_ray_get_angle());
   view_cos_fov = (view_cos*view_fov_factor_x)/1024;
   view_sin_fov = (view_sin*view_fov_factor_x)/1024;
   view_middle_row = RVR_YRES/2+RvR_ray_get_shear();*/
}

void sprite_draw(RvR_fix16 x, RvR_fix16 y, RvR_fix16 z, RvR_fix16 dir, int32_t sprite)
{
   const Sprite *sp = sprites+sprite;
   RvR_ray_draw_sprite(&player.cam,x,y,z,dir,sp->tex[0],0);
   //void               RvR_ray_draw_sprite(const RvR_ray_cam *cam, RvR_fix16 x, RvR_fix16 y, RvR_fix16 z, RvR_fix16 dir, uint16_t sprite, uint32_t flags);
   /*if(sprite<0||sprite>UINT16_MAX)
      return;

   const Sprite *sp = sprites+sprite;
   uint8_t index = 0;
   Sprite_draw sprite_new = {0};

   if(sp->rot)
   {
      RvR_fix22 rot = RvR_fix22_atan2(player.entity->pos.x-pos.x,player.entity->pos.y-pos.y);
      rot = (rot+256-dir)&4095;
      index = (rot/512)&7;
   }

   //Flagged invisible
   if(sp->flag[index]&1)
      return;

   //Translate sp to world space coordinates
   RvR_fix22 half_width = (RvR_texture_get(sp->tex[index])->width*1024/64)/2;
   RvR_vec2 p0,p1;
   p0.x = (-view_sin*half_width)/1024+pos.x;
   p0.y = (view_cos*half_width)/1024+pos.y;
   p1.x = (view_sin*half_width)/1024+pos.x;
   p1.y = (-view_cos*half_width)/1024+pos.y;
   sprite_new.p = pos;
   sprite_new.texture = sp->tex[index];

   //Translate to camera space
   RvR_fix22 x0 = p0.x-RvR_ray_get_position().x;
   RvR_fix22 y0 = p0.y-RvR_ray_get_position().y;
   RvR_fix22 x1 = p1.x-RvR_ray_get_position().x;
   RvR_fix22 y1 = p1.y-RvR_ray_get_position().y;
   RvR_vec2 to_point0;
   RvR_vec2 to_point1;
   to_point0.x = (-x0*view_sin+y0*view_cos)/1024; 
   to_point0.y = (x0*view_cos_fov+y0*view_sin_fov)/1024; 
   to_point1.x = (-x1*view_sin+y1*view_cos)/1024; 
   to_point1.y = (x1*view_cos_fov+y1*view_sin_fov)/1024; 

   //Behind camera
   if(to_point0.y<-128&&to_point1.y<-128)
      return;

   //Sprite not facing camera
   //--> swap p0 and p1
   if((to_point0.x*to_point1.y-to_point1.x*to_point0.y)/65536>0)
   {
      RvR_vec2 tmp = to_point0;
      to_point0 = to_point1;
      to_point1 = tmp;
   }

   //Here we can treat everything as if we have a 90 degree
   //fov, since the rotation to camera space transforms it to
   //that
   //Check if in fov
   //Left point in fov
   if(to_point0.x>=-to_point0.y)
   {
      if(to_point0.x>to_point0.y)
         return;

      //sprite_new.sp0.x = RvR_min(RVR_XRES/2+(to_point0.x*(RVR_XRES/2))/to_point0.y,RVR_XRES-1);
   }
   //Left point to the left of fov
   else
   {
      if(to_point1.x<-to_point1.y)
         return;

      //sprite_new.sp0.x = 0;
   }

   //Right point in fov
   if(to_point1.x<=to_point1.y)
   {
      if(to_point1.x<-to_point1.y)
         return;

      //sprite_new.sp1.x = RvR_min(RVR_XRES/2+(to_point1.x*(RVR_XRES/2))/to_point1.y-1,RVR_XRES-1);
   }
   else
   {
      if(to_point0.x>to_point0.y)
         return;
      //sprite_new.sp1.x = RVR_XRES-1;
   }

   //Project to screen
   RvR_ray_pixel_info p = RvR_ray_map_to_screen(sprite_new.p);
   sprite_new.depth = p.depth;

   //Clipping
   //Behind camera
   if(sprite_new.depth<=0)
      return;
   //Too far away
   if(sprite_new.depth>RVR_RAY_MAX_STEPS*1024)
      return;

   sprite_stack_push(&sprite_new);*/
}

void sprite_draw_end()
{
   /*//Sort sprites
   qsort(sprite_stack.data,sprite_stack.data_used,sizeof(*sprite_stack.data),sprite_cmp);

   //Draw sprites
   for(int i = 0;i<sprite_stack.data_used;i++)
   {
      Sprite_draw *sp = &sprite_stack.data[i];

      RvR_texture *texture = RvR_texture_get(sp->texture);

      RvR_fix22 tpx = sp->p.x-RvR_ray_get_position().x;
      RvR_fix22 tpy = sp->p.y-RvR_ray_get_position().y;
      RvR_fix22 depth = (tpx*view_cos+tpy*view_sin)/1024;
      tpx = (tpx*view_sin-tpy*view_cos)/1024;

      //Dimensions
      RvR_fix22 top = ((sp->p.z-RvR_ray_get_position().z+texture->height*16)*1024)/RvR_non_zero((depth*view_fov_factor_y)/1024);
      top = view_middle_row*1024-top*RVR_YRES;
      int y0 = (top+1023)/1024;

      RvR_fix22 bot = ((sp->p.z-RvR_ray_get_position().z)*1024)/RvR_non_zero((depth*view_fov_factor_y)/1024);
      bot = view_middle_row*1024-bot*RVR_YRES;
      int y1 = (bot-1)/1024;

      RvR_fix22 left = ((tpx+texture->width*8)*1024)/RvR_non_zero((depth*view_fov_factor_x)/1024);
      left = RVR_XRES*512-left*(RVR_XRES/2);
      int x0 = (left+1023)/1024;

      RvR_fix22 right = ((tpx-texture->width*8)*1024)/RvR_non_zero((depth*view_fov_factor_x)/1024);
      right = RVR_XRES*512-right*(RVR_XRES/2);
      int x1 = (right-1)/1024;

      //Floor and ceiling clip
      RvR_fix22 cy = ((RvR_ray_map_floor_height_at(sp->p.x/1024,sp->p.y/1024)-RvR_ray_get_position().z)*1024)/RvR_non_zero((depth*view_fov_factor_y)/1024);
      cy = view_middle_row*1024-cy*RVR_YRES;
      int clip_bottom = RvR_min(cy/1024,RVR_YRES);

      cy = ((RvR_ray_map_ceiling_height_at(sp->p.x/1024,sp->p.y/1024)-RvR_ray_get_position().z)*1024)/RvR_non_zero((depth*view_fov_factor_y)/1024);
      cy = view_middle_row*1024-cy*RVR_YRES;
      int clip_top = RvR_max(cy/1024,0);

      y0 = RvR_max(y0,clip_top);
      y1 = RvR_min(y1,clip_bottom);
      x1 = RvR_min(x1,RVR_XRES);
      RvR_fix22 step_v = (4*view_fov_factor_y*depth)/RVR_YRES;
      RvR_fix22 step_u = (8*view_fov_factor_x*depth)/RVR_XRES;
      RvR_fix22 u = (step_u*(x0*1024-left))/1024;

      if(x0<0)
      {
         u+=(-x0)*step_u;
         x0 = 0;
      }

      //Draw
      const uint8_t * restrict col = RvR_shade_table(RvR_min(63,depth>>9));
      uint8_t * restrict dst = NULL;
      const uint8_t * restrict tex = NULL;
      for(int x = x0;x<x1;x++)
      {
         //Clip against walls
         int ys = y0;
         int ye = y1;

         //Clip floor
         RvR_ray_depth_buffer_entry *clip = RvR_ray_draw_depth_buffer()->floor[x];
         while(clip!=NULL)
         {
            if(depth>clip->depth&&ye>clip->limit)
               ye = clip->limit;
            clip = clip->next;
         }

         //Clip ceiling
         clip = RvR_ray_draw_depth_buffer()->ceiling[x];
         while(clip!=NULL)
         {
            if(depth>clip->depth&&ys<clip->limit)
               ys = clip->limit;
            clip = clip->next;
         }

         tex = &texture->data[texture->height*(u>>16)];
         dst = &RvR_core_framebuffer()[ys*RVR_XRES+x];
         RvR_fix16 v = (sp->p.z-RvR_ray_get_position().z)*4096+(ys-view_middle_row+1)*step_v+texture->height*65536;

         if(sp->flags&32)
         {
            for(int y = ys;y<ye;y++,dst+=RVR_XRES)
            {
               uint8_t index = tex[v>>16];
               *dst = RvR_blend(col[index],*dst);
               v+=step_v;
            }
         }
         else if(sp->flags&64)
         {
            for(int y = ys;y<ye;y++,dst+=RVR_XRES)
            {
               uint8_t index = tex[v>>16];
               *dst = RvR_blend(*dst,col[index]);
               v+=step_v;
            }
         }
         else
         {
            for(int y = ys;y<ye;y++,dst+=RVR_XRES)
            {
               uint8_t index = tex[v>>16];
               *dst = index?col[index]:*dst;
               v+=step_v;
            }
         }

         u+=step_u;
      }
   }*/
}

static void sprite_stack_push(const Sprite_draw *s)
{
   if(sprite_stack.data==NULL)
   {
      sprite_stack.data_size = 64;
      sprite_stack.data = RvR_malloc(sizeof(*sprite_stack.data)*sprite_stack.data_size,"Sprite stack");
      sprite_stack.data_proxy = RvR_malloc(sizeof(*sprite_stack.data_proxy)*sprite_stack.data_size,"Sprite stack proxy");
   }

   sprite_stack.data_proxy[sprite_stack.data_used] = sprite_stack.data_used;
   sprite_stack.data[sprite_stack.data_used++] = *s;

   if(sprite_stack.data_used==sprite_stack.data_size)
   {
      sprite_stack.data_size+=64;
      sprite_stack.data = RvR_realloc(sprite_stack.data,sizeof(*sprite_stack.data)*sprite_stack.data_size,"Sprite stack data grow");
      sprite_stack.data_proxy = RvR_realloc(sprite_stack.data_proxy,sizeof(*sprite_stack.data_proxy)*sprite_stack.data_size,"Sprite stack data_proxy grow");
   }
}

static int sprite_cmp(const void *a, const void *b)
{
   const Sprite_draw *sa = a;
   const Sprite_draw *sb = b;

   return sb->depth-sa->depth;
}
//-------------------------------------
