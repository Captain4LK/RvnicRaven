/*
RvnicRaven-ray

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_ray_config.h"
#include "RvR/RvR_ray.h"
#include "RvR_ray_render.h"
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

void ray_sprite_draw_billboard(const ray_sprite *sp, RvR_ray_selection *select)
{
   RvR_fix16 cos = RvR_fix16_cos(ray_cam->dir);
   RvR_fix16 sin = RvR_fix16_sin(ray_cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(ray_cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() << 16);
   RvR_fix16 middle_row = (RvR_yres() / 2) + ray_cam->shear;

   RvR_texture *texture = RvR_texture_get(sp->texture);

   RvR_fix16 tpx = sp->x - ray_cam->x;
   RvR_fix16 tpy = sp->y - ray_cam->y;
   RvR_fix16 depth = RvR_fix16_mul(tpx, cos) + RvR_fix16_mul(tpy, sin);
   tpx = RvR_fix16_mul(-tpx, sin) + RvR_fix16_mul(tpy, cos);

   //Dimensions
   RvR_fix16 top = middle_row * 65536 - RvR_fix16_div(RvR_yres() * (sp->z - ray_cam->z + texture->height * 1024), RvR_non_zero(RvR_fix16_mul(depth, fovy)));
   int y0 = (top + 65535) / 65536;

   RvR_fix16 bot = middle_row * 65536 - RvR_fix16_div(RvR_yres() * (sp->z - ray_cam->z), RvR_non_zero(RvR_fix16_mul(depth, fovy)));
   int y1 = (bot - 1) / 65536;

   RvR_fix16 left = RvR_xres() * 32768 + RvR_fix16_div((RvR_xres() / 2) * (tpx - texture->width * 512), RvR_non_zero(RvR_fix16_mul(depth, fovx)));
   int x0 = (left + 65535) / 65536;

   RvR_fix16 right = RvR_xres() * 32768 + RvR_fix16_div((RvR_xres() / 2) * (tpx + texture->width * 512), RvR_non_zero(RvR_fix16_mul(depth, fovx)));
   int x1 = (right - 1) / 65536;

   //Floor and ceiling clip
   RvR_fix16 cy = middle_row * 65536 - RvR_fix16_div(RvR_yres() * (RvR_ray_map_floor_height_at(ray_map, (int16_t)(sp->x / 65536), (int16_t)(sp->y / 65536)) - ray_cam->z), RvR_non_zero(RvR_fix16_mul(depth, fovy)));
   int clip_bottom = RvR_min(cy / 65536, RvR_yres());
   y1 = RvR_min(y1, clip_bottom);

   cy = middle_row * 65536 - RvR_fix16_div(RvR_yres() * (RvR_ray_map_ceiling_height_at(ray_map, (int16_t)(sp->x / 65536), (int16_t)(sp->y / 65536)) - ray_cam->z), RvR_non_zero(RvR_fix16_mul(depth, fovy)));
   int clip_top = RvR_max(cy / 65536, 0);
   y0 = RvR_max(y0, clip_top);

   x1 = RvR_min(x1, RvR_xres());
   RvR_fix16 step_v = RvR_fix16_mul(fovy, depth) / RvR_yres();
   RvR_fix16 step_u = RvR_fix16_mul(2 * fovx, depth) / RvR_xres();
   RvR_fix16 u = RvR_fix16_mul(step_u, x0 * 65536 - left);

   if(x0<0)
   {
      u += (-x0) * step_u;
      x0 = 0;
      left = 0;
   }

   //Adjust for fractional part
   RvR_fix16 xfrac = left - x0 * 65536;
   u -= RvR_fix16_mul(xfrac, step_u);

   //Vertical flip
   if(sp->flags & 4)
      step_v = -step_v;

   //Draw
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_min(63, depth >> 15));
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;
   for(int x = x0; x<x1; x++)
   {
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = ray_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      int tu = u / 1024;
      if(sp->flags & 2)
         tu = texture->width - tu - 1;
      tex = &texture->data[texture->height * (tu)];
      dst = &RvR_framebuffer()[ys * RvR_xres() + x];

      RvR_fix16 v = (sp->z - ray_cam->z) + (ys - middle_row + 1) * step_v + texture->height * 1024;
      if(sp->flags & 4)
         v = texture->height * 1024 - ((sp->z - ray_cam->z) + (ys - middle_row + 1) * (-step_v) + texture->height * 1024);

      if(select!=NULL&&select->x==x&&select->y>=ys&&select->y<ye&&select->depth>depth)
      {
         //Check for transparent pixels
         if(tex[(v + step_v * (select->y - ys)) >> 10])
         {
            select->type = RVR_RAY_SSPRITE_BILL;
            select->depth = depth;
            select->ref = sp->ref;
         }
      }

      if(sp->flags & 32)
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 10)];
            *dst = RvR_blend(col[index], *dst);
            v += step_v;
         }
      }
      else if(sp->flags & 64)
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 10)];
            *dst = RvR_blend(*dst, col[index]);
            v += step_v;
         }
      }
      else
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 10)];
            *dst = index?col[index]:*dst;
            v += step_v;
         }
      }

      u += step_u;
   }
}
//-------------------------------------
