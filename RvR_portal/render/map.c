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
#include "RvR_port_plane.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   RvR_fix22 x0;
   RvR_fix22 xw0;
   RvR_fix22 z0;
   RvR_fix22 zw0;
   RvR_fix22 x1;
   RvR_fix22 xw1;
   RvR_fix22 z1;
   RvR_fix22 zw1;
   RvR_fix22 zfront;

   RvR_fix22 u0;
   RvR_fix22 u1;

   RvR_fix22 p0x;
   RvR_fix22 p0y;
   RvR_fix22 p1x;
   RvR_fix22 p1y;

   uint16_t wall;
   uint16_t sector;
}RvR_port_dwall;
//-------------------------------------

//Variables
static uint16_t *sector_stack = NULL;
static RvR_port_dwall *dwalls = NULL;

static RvR_fix22 port_ytop[RVR_XRES_MAX] = {0};
static RvR_fix22 port_ybot[RVR_XRES_MAX] = {0};
//-------------------------------------

//Function prototypes
static int dwall_can_front(const RvR_port_dwall *wa, const RvR_port_dwall *wb);
static void port_collect_walls(uint16_t start);
//-------------------------------------

//Function implementations

void RvR_port_draw_map(RvR_port_selection *select)
{
   RvR_array_length_set(dwalls,0);
   if(select!=NULL)
   {
      select->depth = INT32_MAX;
      select->type = RVR_PORT_NONE;
   }

   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov/2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres()*fovx,RvR_xres()*1024);

   for(int i = 0;i<RvR_xres();i++)
   {
      port_ytop[i] = -1;
      port_ybot[i] = RvR_yres();
   }

   //Start with sector of camera
   int columns_left = RvR_xres();
   port_collect_walls(port_cam->sector);
   while(RvR_array_length(dwalls)>0)
   {
      if(columns_left==0)
         break;

      int len = (int)RvR_array_length(dwalls);
      port_report.stack_max = RvR_max(port_report.stack_max,len);
      {
         int swaps = 0;
         int j = 0+1;
         while(j<len)
         {
            if(dwall_can_front(dwalls+0,dwalls+j))
            {
               j++;
            }
            else if(0+swaps>j)
            {
               //This case usually happens when walls intersect
               //Here we would split the wall, 
               //but since intersecting walls aren't supported we just pretend nothing happended
               j++;
               port_report.sort_skips++;
            }
            else
            {
               RvR_port_dwall tmp = dwalls[j];
               for(int w = j;w>0;w--)
                  dwalls[w] = dwalls[w-1];
               dwalls[0] = tmp;
               j = 0+1;
               swaps++;
               port_report.sort_swaps++;
            }
         }
      }

      RvR_port_dwall tmp = dwalls[0];
      for(int w = 0;w<len-1;w++)
         dwalls[w] = dwalls[w+1];
      RvR_port_dwall *wall = &tmp;
      RvR_array_length_set(dwalls,RvR_array_length(dwalls)-1);
      uint16_t sector = wall->sector;
      uint16_t portal = port_map->walls[wall->wall].portal;

      //Shift texture x coordinates
      wall->u0+=((int32_t)port_map->walls[wall->wall].x_off)*1024;
      wall->u1+=((int32_t)port_map->walls[wall->wall].x_off)*1024;

      //No portal
      if(portal==RVR_PORT_SECTOR_INVALID)
      {
         RvR_fix22 cz0 = port_map->sectors[sector].ceiling;
         RvR_fix22 cz1 = port_map->sectors[sector].ceiling;
         RvR_fix22 fz0 = port_map->sectors[sector].floor;
         RvR_fix22 fz1 = port_map->sectors[sector].floor;

         //Slopes
         //-------------------------------------
         if(port_map->sectors[sector].slope_floor!=0)
         {
            RvR_port_slope slope;
            RvR_port_slope_from_floor(port_map,sector,&slope);
            uint16_t w0 = wall->wall;
            uint16_t w1 = port_map->walls[wall->wall].p2;
            //fz0 = RvR_port_slope_height_at(&slope,port_map->walls[w0].x,port_map->walls[w0].y);
            //fz1 = RvR_port_slope_height_at(&slope,port_map->walls[w1].x,port_map->walls[w1].y);
            fz0 = RvR_port_slope_height_at(&slope,wall->p0x,wall->p0y);
            fz1 = RvR_port_slope_height_at(&slope,wall->p1x,wall->p1y);
         }

         if(port_map->sectors[sector].slope_ceiling!=0)
         {
            RvR_port_slope slope;
            RvR_port_slope_from_ceiling(port_map,sector,&slope);
            uint16_t w0 = wall->wall;
            uint16_t w1 = port_map->walls[wall->wall].p2;
            cz0 = RvR_port_slope_height_at(&slope,wall->p0x,wall->p0y);
            cz1 = RvR_port_slope_height_at(&slope,wall->p1x,wall->p1y);
         }
         //-------------------------------------

         //Top of wall
         RvR_fix22 cy0 = RvR_fix22_div(2*RvR_yres()*(cz0-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 cy1 = RvR_fix22_div(2*RvR_yres()*(cz1-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         cy0 = 4*RvR_yres()*512-cy0;
         cy1 = 4*RvR_yres()*512-cy1;
         RvR_fix22 step_cy = RvR_fix22_div(cy1-cy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 cy = cy0;

         //Bottom of wall
         RvR_fix22 fy0 = RvR_fix22_div(2*RvR_yres()*(fz0-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 fy1 = RvR_fix22_div(2*RvR_yres()*(fz1-port_cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         fy0 = 4*RvR_yres()*512-fy0;
         fy1 = 4*RvR_yres()*512-fy1;
         RvR_fix22 step_fy = RvR_fix22_div(fy1-fy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 fy = fy0;

         //normalize texture coordinates, prevent overflows during interpolation
         //u0 will always be smaller than u1 and at least zero
         RvR_texture *texture = RvR_texture_get(port_map->walls[wall->wall].tex_lower);
         RvR_fix22 u0 = wall->u0-(wall->u0&(~(1024*texture->width-1)));
         RvR_fix22 u1 = wall->u1-(wall->u0&(~(1024*texture->width-1)));

         //Instead of interpolating 1/z and u*z we bring it to the denominator (z1*z0) and
         //interpolate the numerator instead
         RvR_fix22 denom = RvR_fix22_mul(2*wall->z1,2*wall->z0);
         RvR_fix22 num_step_z = 4*(wall->z0-wall->z1);
         RvR_fix22 num_z = 4*wall->z1;
         int64_t num_step_u = RvR_fix22_mul(wall->z0,u1)-RvR_fix22_mul(wall->z1,u0);
         int64_t num_u = ((int64_t)u0*wall->z1)/1024;

         //Adjust for fractional part
         int x0 = (wall->x0+1023)/1024;
         int x1 = (wall->x1+1023)/1024;
         RvR_fix22 xfrac = x0*1024-wall->x0;
         cy+=RvR_fix22_mul(xfrac,step_cy);
         fy+=RvR_fix22_mul(xfrac,step_fy);
         num_z+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(wall->x1-wall->x0));
         num_u+=(xfrac*num_step_u)/RvR_non_zero(wall->x1-wall->x0);

         for(int x = x0;x<x1;x++)
         {
            int y0 = (cy+4095)/4096;
            int y1 = fy/4096;
            int can_write = !(port_ytop[x]>port_ybot[x]);
            RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            int64_t nu = num_u+((num_step_u*(x-x0))*1024)/RvR_non_zero(wall->x1-wall->x0);
            RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz));
            RvR_fix22 u = (RvR_fix22)((4*nu)/RvR_non_zero(nz));

            int top = port_ytop[x]+1;
            y0 = RvR_max(y0,top);
            int bottom = y0-1;
            if(bottom>=port_ybot[x])
               bottom = port_ybot[x]-1;

            if(top<=bottom)
            {
               if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
               {
                  select->type = RVR_PORT_CEILING;
                  select->as.sector = wall->sector;
                  select->depth = depth;
               }

               port_plane_add(wall->sector,0,x,top,bottom);
            }

            bottom = port_ybot[x]-1;
            y1 = RvR_min(y1,bottom);

            top = RvR_max(y1,port_ytop[x])+1;
            if(top<=bottom)
            {
               if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
               {
                  select->type = RVR_PORT_FLOOR;
                  select->as.sector = wall->sector;
                  select->depth = depth;
               }

               port_plane_add(wall->sector,1,x,top,bottom);
            }

            port_ytop[x] = RvR_yres();
            port_ybot[x] = -1;

            if(y0<=y1)
            {
               if(select!=NULL&&select->x==x&&select->y>=y0&&select->y<=y1&&select->depth>depth)
               {
                  select->type = RVR_PORT_WALL_BOT;
                  select->as.wall = wall->wall;
                  select->depth = depth;
               }

               RvR_fix22 height = port_map->sectors[wall->sector].ceiling-port_cam->z;
               RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
               RvR_fix22 texture_coord_scaled = height*4096+(y0-RvR_yres()/2)*coord_step_scaled+((int32_t)port_map->walls[wall->wall].y_off)*65536;
               RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;
               const uint8_t * restrict tex = &texture->data[(((uint32_t)u)%texture->width)*texture->height];
               const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>12)+port_map->walls[wall->wall].shade_offset)));
               uint8_t * restrict pix = RvR_framebuffer()+(y0*RvR_xres()+x);
               for(int y = y0;y<=y1;y++)
               {
                  *pix= col[tex[(texture_coord_scaled>>16)&y_and]];
                  pix+=RvR_xres();
                  texture_coord_scaled+=coord_step_scaled;

#if RVR_PORT_PIXELBYPIXEL
                  if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                     RvR_render_present();
#endif
               }
            }

            cy+=step_cy;
            fy+=step_fy;

            //Depth buffer
            RvR_port_depth_buffer_entry *entry = port_depth_buffer_entry_new();
            entry->depth = depth;
            entry->limit = 0;
            entry->next = port_depth_buffer.floor[x];
            port_depth_buffer.floor[x] = entry;

            if(can_write&&port_ytop[x]>port_ybot[x])
               columns_left--;
         }
      }
      //Portal
      else
      {
         //depth values go much lower here (since you can be on the border between sectors)
         //so we use i64 here
         int64_t cy0 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t cy1 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         cy0 = RvR_yres()*512-cy0;
         cy1 = RvR_yres()*512-cy1;
         int64_t step_cy = ((cy1-cy0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t cy = cy0;

         int64_t cph0 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t cph1 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].ceiling-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         cph0 = RvR_yres()*512-cph0;
         cph1 = RvR_yres()*512-cph1;
         int64_t step_cph = ((cph1-cph0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t cph = cph0;

         int64_t fy0 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t fy1 = ((int64_t)512*RvR_yres()*(port_map->sectors[sector].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         fy0 = RvR_yres()*512-fy0;
         fy1 = RvR_yres()*512-fy1;
         int64_t step_fy = ((fy1-fy0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t fy = fy0;

         int64_t fph0 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z0,fovy));
         int64_t fph1 = ((int64_t)512*RvR_yres()*(port_map->sectors[portal].floor-port_cam->z))/RvR_non_zero(RvR_fix22_mul(wall->z1,fovy));
         fph0 = RvR_yres()*512-fph0;
         fph1 = RvR_yres()*512-fph1;
         int64_t step_fph = ((fph1-fph0)*1024)/RvR_non_zero(wall->x1-wall->x0);
         int64_t fph = fph0;

         RvR_fix22 denom = RvR_fix22_mul(2*wall->z1,2*wall->z0);
         RvR_fix22 num_step_z = 4*(wall->z0-wall->z1);
         RvR_fix22 num_z = 4*wall->z1;

         RvR_texture *texture_lower = RvR_texture_get(port_map->walls[wall->wall].tex_lower);
         RvR_texture *texture_upper = RvR_texture_get(port_map->walls[wall->wall].tex_upper);
         RvR_fix22 u0_lower = wall->u0-(wall->u0&(~(1024*texture_lower->width-1)));
         RvR_fix22 u1_lower = wall->u1-(wall->u0&(~(1024*texture_lower->width-1)));
         RvR_fix22 u0_upper = wall->u0-(wall->u0&(~(1024*texture_upper->width-1)));
         RvR_fix22 u1_upper = wall->u1-(wall->u0&(~(1024*texture_upper->width-1)));
         int64_t num_step_u_lower = RvR_fix22_mul(wall->z0,u1_lower)-RvR_fix22_mul(wall->z1,u0_lower);
         int64_t num_u_lower = ((int64_t)u0_lower*wall->z1)/1024;
         int64_t num_step_u_upper = RvR_fix22_mul(wall->z0,u1_upper)-RvR_fix22_mul(wall->z1,u0_upper);
         int64_t num_u_upper = ((int64_t)u0_upper*wall->z1)/1024;

         //Adjust for fractional part
         int x0 = (wall->x0+1023)/1024;
         int x1 = (wall->x1+1023)/1024;
         RvR_fix22 xfrac = x0*1024-wall->x0;
         cy+=(xfrac*step_cy)/1024;
         cph+=(xfrac*step_cph)/1024;
         fy+=(xfrac*step_fy)/1024;
         fph+=(xfrac*step_fph)/1024;
         num_z+=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(wall->x1-wall->x0));
         num_u_lower+=(xfrac*num_step_u_lower)/RvR_non_zero(wall->x1-wall->x0);
         num_u_upper+=(xfrac*num_step_u_upper)/RvR_non_zero(wall->x1-wall->x0);

         int any_portal = 0;
         int floor_sky = port_map->sectors[wall->sector].flags&RVR_PORT_SECTOR_PARALLAX_FLOOR&
                         port_map->sectors[port_map->walls[wall->wall].portal].flags&RVR_PORT_SECTOR_PARALLAX_FLOOR;
         int ceiling_sky = port_map->sectors[wall->sector].flags&RVR_PORT_SECTOR_PARALLAX_CEILING&&
                           port_map->sectors[port_map->walls[wall->wall].portal].flags&RVR_PORT_SECTOR_PARALLAX_CEILING;
         //ceiling_sky = 0;

         for(int x = x0;x<x1;x++)
         {
            //TODO(Captain4LK): maybe don't cast, but keep i64?
            int y0 = (int)((cy+1023)/1024);
            int y1 = (int)(fy/1024);
            int can_write = !(port_ytop[x]>port_ybot[x]);

            RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            int64_t nu_lower = num_u_lower+((num_step_u_lower*(x-x0))*1024/RvR_non_zero(wall->x1-wall->x0));
            int64_t nu_upper = num_u_upper+((num_step_u_upper*(x-x0))*1024/RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz));
            RvR_fix22 u_lower = (RvR_fix22)((4*nu_lower)/RvR_non_zero(nz));
            RvR_fix22 u_upper = (RvR_fix22)((4*nu_upper)/RvR_non_zero(nz));

            int top = port_ytop[x]+1;
            int bottom = port_ybot[x];

            y0 = RvR_max(y0,top);
            bottom = y0-1;
            if(bottom>=port_ybot[x])
               bottom = port_ybot[x]-1;
            int mid = (int)(cph/1024);
            if(mid>=port_ybot[x])
               mid = port_ybot[x]-1;

            if(ceiling_sky)
            {
               if(top<=RvR_max(mid,bottom))
               {
                  if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
                  {
                     select->type = RVR_PORT_CEILING;
                     select->as.sector = wall->sector;
                     select->depth = depth;
                  }
                  port_plane_add(wall->sector,0,x,top,RvR_max(mid,bottom));
               }
               if(mid>=y0)
               {
                  if(select!=NULL&&select->x==x&&select->y>=y0&&select->y<=mid&&select->depth>depth)
                  {
                     select->type = RVR_PORT_WALL_TOP;
                     select->as.wall = wall->wall;
                     select->depth = depth;
                  }
                  port_ytop[x] = mid;
               }
               else
               {
                  port_ytop[x] = y0-1;
               }
            }
            else
            {
               if(top<=bottom)
               {
                  if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
                  {
                     select->type = RVR_PORT_CEILING;
                     select->as.sector = wall->sector;
                     select->depth = depth;
                  }
                  port_plane_add(wall->sector,0,x,top,bottom);
               }

               const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>12)+port_map->walls[wall->wall].shade_offset)));
               if(mid>=y0)
               {
                  if(select!=NULL&&select->x==x&&select->y>=y0&&select->y<=mid&&select->depth>depth)
                  {
                     select->type = RVR_PORT_WALL_TOP;
                     select->as.wall = wall->wall;
                     select->depth = depth;
                  }

                  RvR_fix22 height = port_map->sectors[portal].ceiling-port_cam->z;
                  RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
                  RvR_fix22 texture_coord_scaled = height*4096+(y0-RvR_yres()/2)*coord_step_scaled+((int32_t)port_map->walls[wall->wall].y_off)*65536;
                  const uint8_t * restrict tex = &texture_upper->data[(((uint32_t)u_upper)%texture_upper->width)*texture_upper->height];
                  uint8_t * restrict pix = RvR_framebuffer()+(y0*RvR_xres()+x);
                  RvR_fix22 y_and = (1<<RvR_log2(texture_upper->height))-1;
                  for(int y = y0;y<=mid;y++)
                  {
                     *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
                     pix+=RvR_xres();
                     texture_coord_scaled+=coord_step_scaled;

#if RVR_PORT_PIXELBYPIXEL
                     if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                        RvR_render_present();
#endif
                  }
                  port_ytop[x] = mid;
               }
               else
               {
                  port_ytop[x] = y0-1;
               }
            }

            int mid_prev = mid;
            bottom = port_ybot[x]-1;
            y1 = RvR_min(y1,bottom);
            top = RvR_max(y1,port_ytop[x])+1;

            mid = (int)((fph+1023)/1024);
            if(mid<=port_ytop[x])
               mid = port_ytop[x]+1;
            
            if(port_ytop[x]<=port_ybot[x]&&mid-mid_prev>0)
               any_portal = 1;

            if(floor_sky)
            {
               if(RvR_min(mid,top)<=bottom)
               {
                  if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
                  {
                     select->type = RVR_PORT_FLOOR;
                     select->as.sector = wall->sector;
                     select->depth = depth;
                  }

                  port_plane_add(wall->sector,1,x,RvR_min(mid,top),bottom);
               }

               if(mid<=y1)
               {
                  if(select!=NULL&&select->x==x&&select->y>=mid&&select->y<=y1&&select->depth>depth)
                  {
                     select->type = RVR_PORT_WALL_BOT;
                     select->as.wall = wall->wall;
                     select->depth = depth;
                  }
                  port_ybot[x] = mid;
               }
               else
               {
                  port_ybot[x] = y1+1;
               }
            }
            else
            {
               if(top<=bottom)
               {
                  if(select!=NULL&&select->x==x&&select->y>=top&&select->y<=bottom&&select->depth>depth)
                  {
                     select->type = RVR_PORT_FLOOR;
                     select->as.sector = wall->sector;
                     select->depth = depth;
                  }

                  port_plane_add(wall->sector,1,x,top,bottom);
               }

               const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>12)+port_map->walls[wall->wall].shade_offset)));
               if(mid<=y1)
               {
                  if(select!=NULL&&select->x==x&&select->y>=mid&&select->y<=y1&&select->depth>depth)
                  {
                     select->type = RVR_PORT_WALL_BOT;
                     select->as.wall = wall->wall;
                     select->depth = depth;
                  }

                  RvR_fix22 height = port_map->sectors[portal].floor-port_cam->z;
                  RvR_fix22 coord_step_scaled = (8*fovy*depth)/RvR_yres();
                  RvR_fix22 texture_coord_scaled = height*4096+(mid-RvR_yres()/2)*coord_step_scaled+coord_step_scaled/2+((int32_t)port_map->walls[wall->wall].y_off)*65536;
                  RvR_fix22 y_and = (1<<RvR_log2(texture_lower->height))-1;
                  const uint8_t * restrict tex = &texture_lower->data[(((uint32_t)u_lower)%texture_lower->width)*texture_lower->height];
                  uint8_t * restrict pix = RvR_framebuffer()+(mid*RvR_xres()+x);
                  for(int y = mid;y<=y1;y++)
                  {
                     *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
                     pix+=RvR_xres();
                     texture_coord_scaled+=coord_step_scaled;

#if RVR_PORT_PIXELBYPIXEL
                     if(RvR_key_pressed(RVR_PORT_PIXELKEY))
                        RvR_render_present();
#endif
                  }
                  port_ybot[x] = mid;
               }
               else
               {
                  port_ybot[x] = y1+1;
               }
            }

            //Depth buffer ceiling
            RvR_port_depth_buffer_entry *entry = port_depth_buffer_entry_new();
            entry->depth = depth;
            entry->limit = port_ytop[x];
            entry->next = port_depth_buffer.ceiling[x];
            port_depth_buffer.ceiling[x] = entry;

            //Depth buffer floor
            entry = port_depth_buffer_entry_new();
            entry->depth = depth;
            entry->limit = port_ybot[x];
            entry->next = port_depth_buffer.floor[x];
            port_depth_buffer.floor[x] = entry;

            if(can_write&&port_ytop[x]>port_ybot[x])
               columns_left--;

            cy+=step_cy;
            cph+=step_cph;
            fy+=step_fy;
            fph+=step_fph;
         }

         if(any_portal&&!port_map->sectors[port_map->walls[wall->wall].portal].visited)
            port_collect_walls(port_map->walls[wall->wall].portal);
      }
   }

   port_planes_draw();
}
//Calculates wether wa can be drawn in front of wb
static int dwall_can_front(const RvR_port_dwall *wa, const RvR_port_dwall *wb)
{
   int64_t x00 = wa->xw0;
   int64_t z00 = wa->zw0;
   int64_t x01 = wa->xw1;
   int64_t z01 = wa->zw1;
   int64_t x10 = wb->xw0;
   int64_t z10 = wb->zw0;
   int64_t x11 = wb->xw1;
   int64_t z11 = wb->zw1;

   int64_t dx0 = x01-x00;
   int64_t dz0 = z01-z00;
   int64_t dx1 = x11-x10;
   int64_t dz1 = z11-z10;

   int64_t cross00 = dx0*(z10-z00)-dz0*(x10-x00);
   int64_t cross01 = dx0*(z11-z00)-dz0*(x11-x00);
   int64_t cross10 = dx1*(z00-z10)-dz1*(x00-x10);
   int64_t cross11 = dx1*(z01-z10)-dz1*(x01-x10);

   //wb completely behind wa
   if(RvR_min(z10,z11)>RvR_max(z00,z01))
      return 1;

   //no overlap on screen
   if(wa->x0>=wb->x1||wa->x1<=wb->x0)
      return 1;

   //p0 and p1 of wb behind wa
   if(cross00>=0&&cross01>=0)
      return 1;

   //p0 and p1 of wa in front of wb
   if(cross10<=0&&cross11<=0)
      return 1;
   
   //Need swapping
   return 0;
}

static void port_collect_walls(uint16_t start)
{
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov/2);
   RvR_fix22 sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 cos = RvR_fix22_cos(port_cam->dir);
   //Instead of correcting for fov later, we premultiply
   //here, this allows us to assume that the edges
   //of the view frustumg are lines with a slope of
   //one through the origin
   RvR_fix22 sin_fov = RvR_fix22_mul(sin,fovx);
   RvR_fix22 cos_fov = RvR_fix22_mul(cos,fovx);

   RvR_array_length_set(sector_stack,0);
   RvR_array_push(sector_stack,start);

   port_map->sectors[start].visited = 1;
   while(RvR_array_length(sector_stack)>0)
   {
      uint16_t sector = sector_stack[RvR_array_length(sector_stack)-1];
      RvR_array_length_set(sector_stack,RvR_array_length(sector_stack)-1);

      for(int i = 0;i<port_map->sectors[sector].wall_count;i++)
      {
         RvR_port_wall *w0 = port_map->walls+port_map->sectors[sector].wall_first+i;
         RvR_port_wall *w1 = port_map->walls+w0->p2;
         RvR_port_dwall dw = {0};

         //Translate to camera space
         RvR_fix22 x0 = w0->x-port_cam->x;
         RvR_fix22 y0 = w0->y-port_cam->y;
         RvR_fix22 x1 = w1->x-port_cam->x;
         RvR_fix22 y1 = w1->y-port_cam->y;

         //Rotate to camera space
         //tp0y and tp1y are depth afterwards
         //Could cache here, but I don't think it's worth it
         //TODO(Captain4LK): reuse x0,y0,x1,y1 once the render has been checked properly
         RvR_fix22 tp0x = RvR_fix22_mul(-x0,sin)+RvR_fix22_mul(y0,cos);
         RvR_fix22 tp0y = RvR_fix22_mul(x0,cos_fov)+RvR_fix22_mul(y0,sin_fov);
         RvR_fix22 tp1x = RvR_fix22_mul(-x1,sin)+RvR_fix22_mul(y1,cos);
         RvR_fix22 tp1y = RvR_fix22_mul(x1,cos_fov)+RvR_fix22_mul(y1,sin_fov);
         dw.xw0 = tp0x;
         dw.xw1 = tp1x;
         dw.zw0 = tp0y;
         dw.zw1 = tp1y;

         //Add portals if wall is very slim
         //The 2d cross product is used to check how close the angles of the two lines
         //are. If they are very similar, the value goes towards zero.
         //After the z-division these lines end up very thin, so they might not be
         //rendered at all. The compare with the length of the lines is
         //basically just arbitrary, short lines are less likely to produce rendering artifacts
         //TODO(Captain4LK): recheck this
         //TODO(Captain4LK): maybe don't consider lines outside of view frustum?
         int16_t portal = w0->portal;
         if(portal>=0&&!port_map->sectors[portal].visited)
         {
            int64_t cross = (int64_t)tp0x*tp1y-(int64_t)tp1x*tp0y;
            int64_t len = ((int64_t)tp1x-tp0x)*((int64_t)tp1x-tp0x);
            len+=((int64_t)tp1y-tp0y)*((int64_t)tp1y-tp0y);

            if((cross*cross)/256<=len)
            {
               RvR_array_push(sector_stack,portal);
               port_map->sectors[portal].visited = 1;
            }
         }

         //Behind camera
         if(tp0y<=0&&tp1y<=0)
            continue;

         //Not facing camera --> winding
         //if 2d cross product is less than zero, p0 is to the left of p1
         if(RvR_fix22_mul(tp0x,tp1y)-RvR_fix22_mul(tp1x,tp0y)>0)
            continue;

         //Calculate texture coordinates based on line length
         dw.u0 = 0;
         dw.u1 = RvR_fix22_sqrt(RvR_fix22_mul(w1->x-w0->x,w1->x-w0->x)+RvR_fix22_mul(w1->y-w0->y,w1->y-w0->y))*64-1;
         RvR_fix22 p0x = w0->x;
         RvR_fix22 p0y = w0->y;
         RvR_fix22 p1x = w1->x;
         RvR_fix22 p1y = w1->y;

         //Clipping
         //Left point in fov
         if(tp0x>=-tp0y)
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            //Simply divide by z 
            dw.x0 = RvR_min(RvR_xres()*512+RvR_fix22_div(tp0x*(RvR_xres()/2),RvR_non_zero(tp0y)),RvR_xres()*1024-1);
            dw.z0 = tp0y;
            dw.p0x = p0x;
            dw.p0y = p0y;
         }
         //Left point to the left of fov
         else
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            //Calculate intersection of line with frustum
            //and adjust texture and depth acordingly
            dw.x0 = 0;
            RvR_fix22 dx0 = tp1x-tp0x;
            RvR_fix22 dx1 = tp0x+tp0y;
            dw.u0 = dw.u0 + RvR_fix22_div(RvR_fix22_mul(-tp0x-tp0y,dw.u1-dw.u0),RvR_non_zero(tp1x-tp0x+tp1y-tp0y));
            dw.z0 = RvR_fix22_div(RvR_fix22_mul(dx0,dx1),RvR_non_zero(tp1y-tp0y+tp1x-tp0x))-tp0x;
            dw.p0x = p0x + RvR_fix22_div(RvR_fix22_mul(-tp0x-tp0y,p1x-p0x),RvR_non_zero(tp1x-tp0x+tp1y-tp0y));
            dw.p0y = p0y + RvR_fix22_div(RvR_fix22_mul(-tp0x-tp0y,p1y-p0y),RvR_non_zero(tp1x-tp0x+tp1y-tp0y));
         }

         //Right point in fov
         if(tp1x<=tp1y)
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            //Simply divide by z 
            dw.x1 = RvR_min(RvR_xres()*512+RvR_fix22_div(tp1x*(RvR_xres()/2),RvR_non_zero(tp1y)),RvR_xres()*1024);
            dw.z1 = tp1y;
            dw.p1x = p1x;
            dw.p1y = p1y;
         }
         else
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            //Calculate intersection of line with frustum
            //and adjust texture and depth acordingly
            RvR_fix22 dx0 = tp1x-tp0x;
            RvR_fix22 dx1 = tp0y-tp0x;
            dw.x1 = RvR_xres()*1024;
            dw.z1 = tp0x-RvR_fix22_div(RvR_fix22_mul(dx0,dx1),RvR_non_zero(tp1y-tp0y-tp1x+tp0x));
            dw.u1 = RvR_fix22_div(RvR_fix22_mul(dx1,dw.u1),RvR_non_zero(-tp1y+tp0y+tp1x-tp0x));
            dw.p1x = p0x + RvR_fix22_div(RvR_fix22_mul(dx1,p1x-p0x),RvR_non_zero(-tp1y+tp0y+tp1x-tp0x));
            dw.p1y = p0y + RvR_fix22_div(RvR_fix22_mul(dx1,p1y-p0y),RvR_non_zero(-tp1y+tp0y+tp1x-tp0x));
         }

         //If the wall somehow ends up having a
         //negativ width, skip it
         if(dw.x0>dw.x1)
            continue;

         //Near clip
         if(dw.z0<=0||dw.z1<1)
            continue;

         dw.wall = port_map->sectors[sector].wall_first+(uint16_t)i;
         dw.sector = sector;
         RvR_array_push(dwalls,dw);
      }
   }
}
//-------------------------------------
