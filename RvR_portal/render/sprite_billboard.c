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

void port_sprite_draw_billboard(const RvR_port_map *map, const port_sprite *sp, RvR_port_selection *select)
{
   RvR_fix22 cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres() * fovx, RvR_xres()*1024);
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;

   RvR_texture *texture = RvR_texture_get(sp->texture);

   RvR_fix22 tpx = sp->x - port_cam->x;
   RvR_fix22 tpy = sp->y - port_cam->y;
   RvR_fix22 depth = RvR_fix22_mul(tpx, cos) + RvR_fix22_mul(tpy, sin);
   tpx = RvR_fix22_mul(-tpx, sin) + RvR_fix22_mul(tpy, cos);

   //Dimensions
   RvR_fix22 top;
   if(sp->flags&RVR_PORT_SPRITE_CENTER)
      top = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z + texture->height * 8), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   else
      top = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z + texture->height * 16), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int y0 = (top + 1023) / 1024;

   RvR_fix22 bot;
   if(sp->flags&RVR_PORT_SPRITE_CENTER)
      bot = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z-texture->height*8), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   else
      bot = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (sp->z - port_cam->z), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int y1 = (bot - 1) / 1024;

   RvR_fix22 left = RvR_xres() * 512+ RvR_fix22_div((RvR_xres() / 2) * (tpx - texture->width * 8), RvR_non_zero(RvR_fix22_mul(depth, fovx)));
   int x0 = (left + 1023) / 1024;

   RvR_fix22 right = RvR_xres() * 512+ RvR_fix22_div((RvR_xres() / 2) * (tpx + texture->width * 8), RvR_non_zero(RvR_fix22_mul(depth, fovx)));
   int x1 = (right - 1) / 1024;

   //Floor and ceiling clip
   RvR_fix22 cy = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (map->sectors[sp->sector].floor - port_cam->z), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int clip_bottom = RvR_min(cy / 1024, RvR_yres());
   y1 = RvR_min(y1, clip_bottom);

   cy = middle_row * 1024- RvR_fix22_div((RvR_yres()/2) * (map->sectors[sp->sector].ceiling - port_cam->z), RvR_non_zero(RvR_fix22_mul(depth, fovy)));
   int clip_top = RvR_max(cy / 1024, 0);
   y0 = RvR_max(y0, clip_top);

   x1 = RvR_min(x1, RvR_xres());
   RvR_fix22 step_v = (8*fovy* depth) / RvR_yres();
   RvR_fix22 step_u = (8 * fovx* depth) / RvR_xres();
   RvR_fix22 u = RvR_fix22_mul(step_u, x0 * 1024- left);

   if(x0<0)
   {
      u += (-x0) * step_u;
      x0 = 0;
      left = 0;
   }

   //Vertical flip
   if(sp->flags & RVR_PORT_SPRITE_YFLIP)
      step_v = -step_v;

   //Draw
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_min(63, depth >> 12));
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;
   for(int x = x0; x<x1; x++)
   {
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      RvR_port_depth_buffer_entry *clip = port_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = port_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      int tu = u / 65536;
      if(sp->flags & RVR_PORT_SPRITE_XFLIP)
         tu = texture->width - tu - 1;
      tex = &texture->data[texture->height * (tu)];
      dst = &RvR_framebuffer()[ys * RvR_xres() + x];

      RvR_fix22 v;
      if(sp->flags&RVR_PORT_SPRITE_CENTER)
         v = (sp->z - port_cam->z)*4096 + (ys - middle_row + 1) * step_v + texture->height * 32768;
      else
         v = (sp->z - port_cam->z)*4096 + (ys - middle_row + 1) * step_v + texture->height * 65536;

      if(sp->flags & RVR_PORT_SPRITE_XFLIP)
         v = texture->height * 1024 - ((sp->z - port_cam->z) + (ys - middle_row + 1) * (-step_v) + texture->height * 1024);

      if(select!=NULL&&select->x==x&&select->y>=ys&&select->y<ye&&select->depth>depth)
      {
         //Check for transparent pixels
         if(tex[(v + step_v * (select->y - ys)) >> 16])
         {
            //TODO(Captain4LK): sprite pointer
            select->depth = depth;
            select->type = RVR_PORT_SPRITE_BILL;
         }
      }

      if(sp->flags & RVR_PORT_SPRITE_TRANS0)
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 16)];
            *dst = RvR_blend(col[index], *dst);
            v += step_v;
         }
      }
      else if(sp->flags & RVR_PORT_SPRITE_TRANS1)
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 16)];
            *dst = RvR_blend(*dst, col[index]);
            v += step_v;
         }
      }
      else
      {
         for(int y = ys; y<ye; y++, dst += RvR_xres())
         {
            uint8_t index = tex[(v >> 16)];
            *dst = index?col[index]:*dst;
            v += step_v;
         }
      }

      u += step_u;
   }
}
//-------------------------------------
