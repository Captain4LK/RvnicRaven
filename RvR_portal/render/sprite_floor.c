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
static RvR_fix22 port_span_start[RVR_YRES_MAX];
//-------------------------------------

//Function prototypes
static void port_floor_span_draw(const port_sprite *sp, int x0, int x1, int y, const RvR_texture *texture);
//-------------------------------------

//Function implementations

void port_sprite_draw_floor(const port_sprite *sp, RvR_port_selection *select)
{
   int xres = RvR_xres();
   int yres = RvR_yres();

   //After clipping we will never have more than 8 vertices
   RvR_fix22 verts[8][3];
   RvR_fix22 verts2[8][3];
   int verts_count = 4;
   int verts2_count = 0;
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(yres * fovx, xres*1024);
   RvR_texture *texture = RvR_texture_get(sp->texture);
   RvR_fix22 middle_row = (yres / 2) + port_cam->shear;
   RvR_fix22 middle_row2 = (yres / 2) - port_cam->shear;

   verts[0][0] = sp->as.floor.x0;
   verts[0][1] = sp->as.floor.z0;
   verts[0][2] = (sp->z - port_cam->z) * (yres/2);
   verts[1][0] = sp->as.floor.x1;
   verts[1][1] = sp->as.floor.z1;
   verts[1][2] = (sp->z - port_cam->z) * (yres/2);
   verts[2][0] = sp->as.floor.x2;
   verts[2][1] = sp->as.floor.z2;
   verts[2][2] = (sp->z - port_cam->z) * (yres/2);
   verts[3][0] = sp->as.floor.x3;
   verts[3][1] = sp->as.floor.z3;
   verts[3][2] = (sp->z - port_cam->z) * (yres/2);

   //Clip to view
   //-------------------------------
   //Clip left
   verts2_count = 0;
   RvR_fix22 left = verts[0][0] + verts[0][1];
   for(int i = 0; i<verts_count; i++)
   {
      int p2 = (i + 1) % verts_count;
      RvR_fix22 leftn = verts[p2][0] + verts[p2][1];
      if(left>=0)
      {
         verts2[verts2_count][0] = verts[i][0];
         verts2[verts2_count][1] = verts[i][1];
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }
      if((left ^ leftn)<0)
      {
         verts2[verts2_count][0] = verts[i][0] + RvR_fix22_mul(RvR_fix22_div(left, left - leftn), verts[p2][0] - verts[i][0]);
         verts2[verts2_count][1] = verts[i][1] + RvR_fix22_mul(RvR_fix22_div(left, left - leftn), verts[p2][1] - verts[i][1]);
         verts2[verts2_count][2] = verts[i][2] + RvR_fix22_mul(RvR_fix22_div(left, left - leftn), verts[p2][2] - verts[i][2]);
         //verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }

      left = leftn;
   }
   if(verts2_count<=2)
      return;

   //Clip right
   verts_count = 0;
   RvR_fix22 right = verts2[0][0] - verts2[0][1];
   for(int i = 0; i<verts2_count; i++)
   {
      int p2 = (i + 1) % verts2_count;
      RvR_fix22 rightn = verts2[p2][0] - verts2[p2][1];
      if(right<=0)
      {
         verts[verts_count][0] = verts2[i][0];
         verts[verts_count][1] = verts2[i][1];
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }
      if((right ^ rightn)<0)
      {
         verts[verts_count][0] = verts2[i][0] + RvR_fix22_mul(RvR_fix22_div(right, right - rightn), verts2[p2][0] - verts2[i][0]);
         verts[verts_count][1] = verts2[i][1] + RvR_fix22_mul(RvR_fix22_div(right, right - rightn), verts2[p2][1] - verts2[i][1]);
         verts[verts_count][2] = verts2[i][2] + RvR_fix22_mul(RvR_fix22_div(right, right - rightn), verts2[p2][2] - verts2[i][2]);
         //verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }

      right = rightn;
   }
   if(verts_count<=2)
      return;

   //Clip bottom
   verts2_count = 0;
   RvR_fix22 down = verts[0][2] + RvR_fix22_mul(verts[0][1], fovy) * middle_row2;
   for(int i = 0; i<verts_count; i++)
   {
      int p2 = (i + 1) % verts_count;
      RvR_fix22 downn = verts[p2][2] + RvR_fix22_mul(verts[p2][1], fovy) * middle_row2;

      if(down>=0)
      {
         verts2[verts2_count][0] = verts[i][0];
         verts2[verts2_count][1] = verts[i][1];
         verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }
      if((down ^ downn)<0)
      {
         verts2[verts2_count][0] = verts[i][0] + RvR_fix22_div(RvR_fix22_mul(down, verts[p2][0] - verts[i][0]), down - downn);
         verts2[verts2_count][1] = verts[i][1] + RvR_fix22_div(RvR_fix22_mul(down, verts[p2][1] - verts[i][1]), down - downn);
         verts2[verts2_count][2] = verts[i][2] + RvR_fix22_div(RvR_fix22_mul(down, verts[p2][2] - verts[i][2]), down - downn);
         //verts2[verts2_count][2] = verts[i][2];
         verts2_count++;
      }

      down = downn;
   }
   if(verts2_count<=2)
      return;

   //Clip top
   verts_count = 0;
   RvR_fix22 up = verts2[0][2] + RvR_fix22_mul(verts2[0][1], fovy) * (middle_row2 - yres);
   for(int i = 0; i<verts2_count; i++)
   {
      int p2 = (i + 1) % verts2_count;
      RvR_fix22 upn = verts2[p2][2] + RvR_fix22_mul(verts2[p2][1], fovy) * (middle_row2 - yres);

      if(up<=0)
      {
         verts[verts_count][0] = verts2[i][0];
         verts[verts_count][1] = verts2[i][1];
         verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }
      if((up ^ upn)<0)
      {
         verts[verts_count][0] = verts2[i][0] + RvR_fix22_mul(RvR_fix22_div(up, RvR_non_zero(up - upn)), verts2[p2][0] - verts2[i][0]);
         verts[verts_count][1] = verts2[i][1] + RvR_fix22_mul(RvR_fix22_div(up, RvR_non_zero(up - upn)), verts2[p2][1] - verts2[i][1]);
         verts[verts_count][2] = verts2[i][2] + RvR_fix22_mul(RvR_fix22_div(up, RvR_non_zero(up - upn)), verts2[p2][2] - verts2[i][2]);
         //verts[verts_count][2] = verts2[i][2];
         verts_count++;
      }

      up = upn;
   }
   if(verts_count<=2)
      return;

   //Project to screen
   for(int i = 0; i<verts_count; i++)
   {
      verts[i][0] = RvR_max(0, RvR_min(xres * 1024- 1, xres * 512+ RvR_fix22_div(verts[i][0] * (xres / 2), RvR_non_zero(verts[i][1]))));
      verts[i][2] = RvR_max(0, RvR_min(yres * 1024- 1, middle_row * 1024- RvR_fix22_div(verts[i][2], RvR_non_zero(RvR_fix22_mul(verts[i][1], fovy)))));
   }
   //-------------------------------

   //Rasterize
   //-------------------------------
   int index_minl = 0;
   RvR_fix22 xmin = xres * 1024;
   for(int i = 0; i<verts_count; i++)
   {
      if(verts[i][0]<xmin)
      {
         xmin = verts[i][0];
         index_minl = i;
      }
   }

   int index_minr = index_minl;

   RvR_fix22 le_y = 0;
   RvR_fix22 le_dy = 0;
   RvR_fix22 le_width = 0;
   RvR_fix22 re_y = 0;
   RvR_fix22 re_dy = 0;
   RvR_fix22 re_width = 0;

   int x = (xmin + 1023) / 1024;

   int prev_start = yres;
   int prev_end = 0;
   for(;;)
   {
      if(!le_width)
      {
         int index_rightl = (index_minl - 1 + verts_count) % verts_count;
         le_width = (verts[index_rightl][0] - verts[index_minl][0]) / 1024;
         //le_width = (verts[index_rightl][0] + 1023) / 1024- (verts[index_minl][0] + 1023) / 1024;
         if(le_width<0)
            break;

         le_dy = RvR_fix22_div(verts[index_rightl][2] - verts[index_minl][2], RvR_non_zero(verts[index_rightl][0] - verts[index_minl][0]));
         le_y = verts[index_minl][2] + RvR_fix22_mul(le_dy, ((verts[index_minl][0] + 1023) / 1024) * 1024- verts[index_minl][0]);

         index_minl = index_rightl;
      }

      if(!re_width)
      {
         int index_rightr = (index_minr + 1) % verts_count;
         re_width = (verts[index_rightr][0] - verts[index_minr][0]) / 1024;
         //re_width = (verts[index_rightr][0] + 1023) / 1024- (verts[index_minr][0] + 1023) / 1024;
         if(re_width<0)
            break;

         re_dy = RvR_fix22_div(verts[index_rightr][2] - verts[index_minr][2], RvR_non_zero(verts[index_rightr][0] - verts[index_minr][0]));
         re_y = verts[index_minr][2] + RvR_fix22_mul(re_dy, ((verts[index_minr][0] + 1023) / 1024) * 1024- verts[index_minr][0]);

         index_minr = index_rightr;
      }

      if(!re_width&&!le_width)
         break;

      int width = RvR_min(le_width, re_width);

      le_width -= width;
      re_width -= width;

      while(width-->0)
      {
         int start = RvR_max(0, RvR_min(yres - 1, (le_y + 1023) / 1024));
         int end = RvR_max(0, RvR_min(yres - 1, (re_y + 1023) / 1024));
         if(start>end)
         {
            int tmp = start;
            start = end;
            end = tmp;
         }

         //We just clip to the middle of the sprite,
         //it's fine since floor aligned sprites
         //aren't supposted to intersect walls
         //Clip floor
         RvR_port_depth_buffer_entry *clip = port_depth_buffer.floor[x];
         while(clip!=NULL)
         {
            if(sp->as.floor.wy>clip->depth&&end>clip->limit)
               end = clip->limit;
            clip = clip->next;
         }

         //Clip ceiling
         clip = port_depth_buffer.ceiling[x];
         while(clip!=NULL)
         {
            if(sp->as.floor.wy>clip->depth&&start<clip->limit)
               start = clip->limit;
            clip = clip->next;
         }

         if(select!=NULL)
         {
            if(x==select->x&&select->y>=start&&select->y<=end&&select->depth>sp->as.floor.wy)
            {
               //select->as.sprite = sp->sprite;
               select->depth = sp->as.floor.wy;
               select->type = RVR_PORT_SSPRITE_FLOOR;
            }
         }

         RvR_fix22 s0 = prev_start;
         RvR_fix22 s1 = start;
         RvR_fix22 e0 = prev_end;
         RvR_fix22 e1 = end;

         //End spans top
         for(; s0<s1&&s0<=e0; s0++)
            port_floor_span_draw(sp, port_span_start[s0], x, s0, texture);

         //End spans bottom
         for(; e0>e1&&e0>=s0; e0--)
            port_floor_span_draw(sp, port_span_start[e0], x, e0, texture);

         //Start spans top
         for(; s1<s0&&s1<=e1; s1++)
            port_span_start[s1] = x;

         //Start spans bottom
         for(; e1>e0&&e1>=s1; e1--)
            port_span_start[e1] = x;

         prev_start = start;
         prev_end = end;
         le_y += le_dy;
         re_y += re_dy;
         x++;
      }
   }

   //End remaining
   RvR_fix22 s0 = prev_start;
   RvR_fix22 s1 = prev_end;
   for(; s0<=s1; s0++)
      port_floor_span_draw(sp, port_span_start[s0], x, s0, texture);
   //-------------------------------
}

static void port_floor_span_draw(const port_sprite *sp, int x0, int x1, int y, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   if(texture==NULL)
      return;

   int xres = RvR_xres();
   int yres = RvR_yres();

   RvR_fix22 view_sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 view_cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov / 2);
   RvR_fix22 fovy = RvR_fix22_div(yres * fovx, xres*1024);
   RvR_fix22 middle_row = (yres / 2) + port_cam->shear;

   RvR_fix22 dy = middle_row - y;
   RvR_fix22 depth = RvR_fix22_div(RvR_abs(port_cam->z - sp->z), RvR_non_zero(fovy));
   depth = RvR_fix22_div(depth * (yres/2), RvR_non_zero(RvR_abs(dy)*1024)); //TODO

   RvR_fix22 x_log = RvR_log2(texture->width);
   RvR_fix22 y_log = RvR_log2(texture->height);
   RvR_fix22 step_x = ((sp->y_units*view_sin*(port_cam->z-sp->z))/RvR_non_zero(16*dy));
   RvR_fix22 step_y = ((sp->x_units*view_cos*(port_cam->z-sp->z))/RvR_non_zero(16*dy));
   RvR_fix22 tx = (port_cam->x-sp->x)*64*sp->y_units+(sp->y_units*view_cos*depth)/16+(x0-xres/2)*step_x;
   RvR_fix22 ty = -(port_cam->y-sp->y)*64*sp->x_units-(sp->x_units*view_sin*depth)/16+(x0-xres/2)*step_y;
   RvR_fix22 x_and = (1 << x_log) - 1;
   RvR_fix22 y_and = (1 << y_log) - 1;

   //Rotate texture coordinates according to sprite rotation
   RvR_fix22 sp_sin = RvR_fix22_sin(-sp->dir);
   RvR_fix22 sp_cos = RvR_fix22_cos(-sp->dir);
   RvR_fix22 tmp = tx;
   tx = RvR_fix22_mul(-sp_sin, tx) + RvR_fix22_mul(sp_cos, ty);
   ty = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, ty);
   tmp = step_x;
   step_x = RvR_fix22_mul(-sp_sin, step_x) + RvR_fix22_mul(sp_cos, step_y);
   step_y = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, step_y);

   //Offset texture coordinates
   //since sprites are anchored in their middle
   tx += texture->width * 8192;
   ty += texture->height * 8192;

   uint8_t * restrict pix = RvR_framebuffer() + y * xres + x0;
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0, RvR_min(63, (depth >> 12))));
   const uint8_t * restrict tex = texture->data;

   if(sp->flags & RVR_PORT_SPRITE_TRANS0)
   {
      for(int x = x0; x<x1; x++)
      {
         uint8_t c = tex[(((tx >> 14) & x_and) << y_log) + ((ty >> 14) & y_and)];
         *pix = RvR_blend(col[c], *pix);
         tx += step_x;
         ty += step_y;
         pix++;
      }
   }
   else if(sp->flags & RVR_PORT_SPRITE_TRANS1)
   {
      for(int x = x0; x<x1; x++)
      {
         uint8_t c = tex[(((tx >> 14) & x_and) << y_log) + ((ty >> 14) & y_and)];
         *pix = RvR_blend(*pix, col[c]);
         tx += step_x;
         ty += step_y;
         pix++;
      }
   }
   else
   {
      for(int x = x0; x<x1; x++)
      {
         uint8_t c = tex[(((tx >> 14) & x_and) << y_log) + ((ty >> 14) & y_and)];
         *pix = c?col[c]:*pix;
         tx += step_x;
         ty += step_y;
         pix++;
      }
   }
}
//-------------------------------------
