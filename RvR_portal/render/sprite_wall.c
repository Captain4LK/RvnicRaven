/*
RvnicRaven-portal

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_port_config.h"
#include "RvR/RvR_portal.h"
#include "RvR_port_render.h"
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

void port_sprite_draw_wall(const port_sprite *sp, RvR_port_selection *select)
{
   int xres = RvR_xres();
   int yres = RvR_yres();

   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix22 scale_vertical = (texture->height * 16*16)/RvR_non_zero(sp->y_units); //texture height in map coordinates
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(yres * fovx, xres*1024);
   RvR_fix22 middle_row = (yres / 2) + port_cam->shear;

   RvR_fix22 cy0;
   RvR_fix22 cy1;
   if(sp->flags&RVR_PORT_SPRITE_CENTER)
   {
      RvR_fix22 topz = RvR_min(sp->z+scale_vertical/2,port_map->sectors[sp->sector].ceiling);
      cy0 = RvR_fix22_div((yres/2) * (topz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z0, fovy)));
      cy1 = RvR_fix22_div((yres/2) * (topz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z1, fovy)));
   }
   else
   {
      RvR_fix22 topz = RvR_min(sp->z+scale_vertical,port_map->sectors[sp->sector].ceiling);
      cy0 = RvR_fix22_div((yres/2) * (topz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z0, fovy)));
      cy1 = RvR_fix22_div((yres/2) * (topz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z1, fovy)));
   }
   cy0 = middle_row * 1024- cy0;
   cy1 = middle_row * 1024- cy1;
   RvR_fix22 step_cy = RvR_fix22_div(cy1 - cy0, RvR_non_zero(sp->as.wall.x1 - sp->as.wall.x0));
   RvR_fix22 cy = cy0;

   RvR_fix22 fy0;
   RvR_fix22 fy1;
   if(sp->flags&RVR_PORT_SPRITE_CENTER)
   {
      RvR_fix22 botz = RvR_max(sp->z-scale_vertical/2,port_map->sectors[sp->sector].floor);
      fy0 = RvR_fix22_div((yres/2) * (botz- port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z0, fovy)));
      fy1 = RvR_fix22_div((yres/2) * (botz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z1, fovy)));
   }
   else
   {
      RvR_fix22 botz = RvR_max(sp->z,port_map->sectors[sp->sector].floor);
      fy0 = RvR_fix22_div((yres/2) * (botz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z0, fovy)));
      fy1 = RvR_fix22_div((yres/2) * (botz - port_cam->z), RvR_non_zero(RvR_fix22_mul(sp->as.wall.z1, fovy)));
   }
   fy0 = middle_row * 1024- fy0;
   fy1 = middle_row * 1024- fy1;
   RvR_fix22 step_fy = RvR_fix22_div(fy1 - fy0, RvR_non_zero(sp->as.wall.x1 - sp->as.wall.x0));
   RvR_fix22 fy = fy0;

   //1/z and u/z can be interpolated linearly
   //Instead of actually calculating 1/z (which would be imprecise for fixed point)
   //we bring both 1/z and u/z on the common denominator (z0*z1*w) and interpolate
   //the numerators instead
   RvR_fix22 denom = RvR_fix22_mul(sp->as.wall.z1,sp->as.wall.z0);
   RvR_fix22 num_step_z = 4*(sp->as.wall.z0-sp->as.wall.z1);
   RvR_fix22 num_z = 4*sp->as.wall.z1;

   RvR_fix22 num_step_u = RvR_fix22_mul(sp->as.wall.z0,sp->as.wall.u1)-RvR_fix22_mul(sp->as.wall.z1,sp->as.wall.u0);
   RvR_fix22 num_u = RvR_fix22_mul(sp->as.wall.u0,sp->as.wall.z1);

   //Adjust for fractional part
   int x0 = (sp->as.wall.x0 + 1023)/1024;
   int x1 = (sp->as.wall.x1 + 1023)/1024;
   RvR_fix22 xfrac = sp->as.wall.x0 - x0 * 1024;
   cy -= RvR_fix22_mul(xfrac, step_cy);
   fy -= RvR_fix22_mul(xfrac, step_fy);
   num_z-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
   num_u-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));

   for(int x = x0; x<x1; x++)
   {
      int y0 = (cy + 1023) / 1024;
      int y1 = fy / 1024;

      RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
      RvR_fix22 nu = num_u+RvR_fix22_div(num_step_u*(x-x0),RvR_non_zero(sp->as.wall.x1-sp->as.wall.x0));
      RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz/4));
      RvR_fix22 u = (4*nu)/RvR_non_zero(nz);

      //Clip floor
      int ybot = yres - 1;
      RvR_port_depth_buffer_entry *clip = port_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ybot>clip->limit)
            ybot = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      int ytop = 0;
      clip = port_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ytop<clip->limit)
            ytop = clip->limit;
         clip = clip->next;
      }

      int wy =  ytop;
      uint8_t * restrict pix = RvR_framebuffer() + (wy * xres + x);

      int y_to = RvR_min(y0, ybot);
      if(y_to>wy)
      {
         wy = y_to;
         pix = RvR_framebuffer() + (wy * xres + x);
      }

      //Wall
      //Inverting the texture coordinates directly didn't work properly,
      //so we just invert u here.
      //TODO: investigate this
      if(sp->flags & RVR_PORT_SPRITE_XFLIP)
         u = texture->width - u - 1;
      RvR_fix22 height;
      if(sp->flags&RVR_PORT_SPRITE_CENTER)
         height = sp->z + scale_vertical/2 - port_cam->z;
      else
         height = sp->z + scale_vertical - port_cam->z;
      RvR_fix22 coord_step_scaled = (sp->y_units*fovy*depth)/(yres*2);
      RvR_fix22 texture_coord_scaled = height*256*sp->y_units+(wy-middle_row+1)*coord_step_scaled;
      //Vertical flip
      if(sp->flags & RVR_PORT_SPRITE_YFLIP)
      {
         if(sp->flags&RVR_PORT_SPRITE_CENTER)
            height = sp->z - scale_vertical/2 - port_cam->z;
         else
            height = sp->z - port_cam->z;
         coord_step_scaled = -coord_step_scaled;
         texture_coord_scaled = (16*texture->height * 16)/RvR_non_zero(sp->y_units)- height*256*sp->y_units+ (wy - middle_row + 1) * coord_step_scaled;
      }
      const uint8_t * restrict tex = &texture->data[(((uint32_t)u) % texture->width) * texture->height];
      const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (depth >> 12))));


      y_to = RvR_min(y1, ybot);

      if(select!=NULL&&select->x==x&&select->y>=wy&&select->y<y_to&&select->depth>depth)
      {
         //Check for transparent pixels
         if(tex[(texture_coord_scaled + coord_step_scaled * (select->y - wy)) >> 16])
         {
            select->as.sprite = sp->sprite;
            select->depth = depth;
            select->type = RVR_PORT_SSPRITE_WALL;
         }
      }

      int stride = xres;
      if(sp->flags & RVR_PORT_SPRITE_TRANS0)
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 16)];
            *pix = RvR_blend(col[index], *pix);
            pix += stride;
            texture_coord_scaled += coord_step_scaled;
         }
      }
      else if(sp->flags & RVR_PORT_SPRITE_TRANS1)
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 16)];
            *pix = RvR_blend(*pix, col[index]);
            pix += stride;
            texture_coord_scaled += coord_step_scaled;
         }
      }
      else
      {
         for(; wy<y_to; wy++)
         {
            uint8_t index = tex[(texture_coord_scaled >> 16)];
            *pix = index?col[index]:*pix;
            pix += stride;
            texture_coord_scaled += coord_step_scaled;
         }
      }

      cy += step_cy;
      fy += step_fy;
   }
}
//-------------------------------------
