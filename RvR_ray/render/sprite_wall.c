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

void ray_sprite_draw_wall(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp, RvR_ray_selection *select)
{
   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix16 scale_vertical = texture->height * 1024; //texture height in map coordinates
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() << 16);
   RvR_fix16 middle_row = (RvR_yres() / 2) + cam->shear;

   RvR_fix16 cy0 = RvR_fix16_div(RvR_yres() * (sp->z + scale_vertical - cam->z), RvR_fix16_mul(sp->as.wall.z0, fovy));
   RvR_fix16 cy1 = RvR_fix16_div(RvR_yres() * (sp->z + scale_vertical - cam->z), RvR_fix16_mul(sp->as.wall.z1, fovy));
   cy0 = middle_row * 65536 - cy0;
   cy1 = middle_row * 65536 - cy1;
   RvR_fix16 step_cy = RvR_fix16_div(cy1 - cy0, RvR_non_zero(sp->as.wall.x1 - sp->as.wall.x0));
   RvR_fix16 cy = cy0;

   RvR_fix16 fy0 = RvR_fix16_div(RvR_yres() * (sp->z - cam->z), RvR_fix16_mul(sp->as.wall.z0, fovy));
   RvR_fix16 fy1 = RvR_fix16_div(RvR_yres() * (sp->z - cam->z), RvR_fix16_mul(sp->as.wall.z1, fovy));
   fy0 = middle_row * 65536 - fy0;
   fy1 = middle_row * 65536 - fy1;
   RvR_fix16 step_fy = RvR_fix16_div(fy1 - fy0, RvR_non_zero(sp->as.wall.x1 - sp->as.wall.x0));
   RvR_fix16 fy = fy0;

   //1/z and u/z can be interpolated linearly
   //Instead of actually calculating 1/z (which would be imprecise for fixed point)
   //we bring both 1/z and u/z on the common denominator (z0*z1*w) and interpolate
   //the numerators instead
   RvR_fix16 denom = RvR_fix16_mul(sp->as.wall.x1 - sp->as.wall.x0, RvR_fix16_mul(sp->as.wall.z1, sp->as.wall.z0));
   RvR_fix16 num_step_z = (sp->as.wall.z0 - sp->as.wall.z1);
   RvR_fix16 num_z = RvR_fix16_mul(sp->as.wall.z1, sp->as.wall.x1 - sp->as.wall.x0);

   RvR_fix16 num_step_u = (RvR_fix16_mul(sp->as.wall.z0, sp->as.wall.u1) - RvR_fix16_mul(sp->as.wall.z1, sp->as.wall.u0));
   RvR_fix16 num_u = RvR_fix16_mul(sp->as.wall.x1 - sp->as.wall.x0, RvR_fix16_mul(sp->as.wall.u0, sp->as.wall.z1));

   //We don't need as much precision
   //and this prevents overflow
   //switch to i64 if you want to change this
   num_z >>= 4;
   num_u >>= 4;
   num_step_z >>= 4;
   num_step_u >>= 4;
   denom >>= 4;

   //printf("%d %d\n",num_u,num_step_u);

   //Adjust for fractional part
   int x0 = (sp->as.wall.x0 + 65535) >> 16;
   int x1 = (sp->as.wall.x1 + 65535) >> 16;
   RvR_fix16 xfrac = sp->as.wall.x0 - x0 * 65536;
   cy -= RvR_fix16_mul(xfrac, step_cy);
   fy -= RvR_fix16_mul(xfrac, step_fy);
   num_z -= RvR_fix16_mul(xfrac, num_step_z);
   num_u -= RvR_fix16_mul(xfrac, num_step_u);

   for(int x = x0; x<x1; x++)
   {
      RvR_fix16 depth = RvR_fix16_div(denom, RvR_non_zero(num_z));

      int y0 = (cy + 65535) / 65536;
      int y1 = fy / 65536;

      //Clip floor
      int ybot = RvR_yres() - 1;
      RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ybot>clip->limit)
            ybot = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      int ytop = 0;
      clip = ray_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ytop<clip->limit)
            ytop = clip->limit;
         clip = clip->next;
      }

      int wy =  ytop;
      uint8_t * restrict pix = RvR_framebuffer() + (wy * RvR_xres() + x);

      int y_to = RvR_min(y0, ybot);
      if(y_to>wy)
      {
         wy = y_to;
         pix = RvR_framebuffer() + (wy * RvR_xres() + x);
      }

      //Wall
      RvR_fix16 u = num_u / RvR_non_zero(num_z);
      //Inverting the texture coordinates directly didn't work properly,
      //so we just invert u here.
      //TODO: investigate this
      if(sp->flags & 2)
         u = texture->width - u - 1;
      RvR_fix16 height = sp->z + scale_vertical - cam->z;
      RvR_fix16 coord_step_scaled = RvR_fix16_mul(fovy, depth) / RvR_yres();
      RvR_fix16 texture_coord_scaled = height + (wy - middle_row + 1) * coord_step_scaled;
      //Vertical flip
      if(sp->flags & 4)
      {
         coord_step_scaled = -coord_step_scaled;
         texture_coord_scaled = texture->height * 1024 - height + (wy - middle_row + 1) * coord_step_scaled;
      }
      const uint8_t * restrict tex = &texture->data[(((uint32_t)u) % texture->width) * texture->height];
      const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (depth >> 15))));


      y_to = RvR_min(y1, ybot);

      if(select!=NULL&&select->x==x&&select->y>=wy&&select->y<y_to&&select->depth>depth)
      {
         //Check for transparent pixels
         if(tex[(texture_coord_scaled + coord_step_scaled * (select->y - wy)) >> 10])
         {
            select->depth = depth;
            select->ref = sp->ref;
         }
      }

      if(sp->flags & 32)
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 10)];
            *pix = RvR_blend(col[index], *pix);
            pix += RvR_xres();
            texture_coord_scaled += coord_step_scaled;
         }
      }
      else if(sp->flags & 64)
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 10)];
            *pix = RvR_blend(*pix, col[index]);
            pix += RvR_xres();
            texture_coord_scaled += coord_step_scaled;
         }
      }
      else
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 10)];
            *pix = index?col[index]:*pix;
            pix += RvR_xres();
            texture_coord_scaled += coord_step_scaled;
         }
      }

      cy += step_cy;
      fy += step_fy;
      num_z += num_step_z;
      num_u += num_step_u;
   }
}
//-------------------------------------
