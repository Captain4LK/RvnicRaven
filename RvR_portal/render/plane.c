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
   RvR_fix22 u0,u1,u2;
   RvR_fix22 v0,v1,v2;
   RvR_fix22 z0,z1,z2;
}port_plane_ctx;
//-------------------------------------

//Variables
static port_plane *port_plane_pool = NULL;
static RvR_fix22 port_span_start[RVR_YRES_MAX];
static port_plane *port_planes[128];
//-------------------------------------

//Function prototypes
static port_plane *port_plane_new();
//static void port_span_draw(const RvR_port_map *map, const RvR_port_cam *cam, int16_t sector, uint8_t where, int x0, int x1, int y);

static void port_plane_sky(port_plane *pl);
static void port_plane_flat(port_plane *pl);
static void port_plane_slope(port_plane *pl);
static void port_span_flat(int16_t sector, uint8_t where, int x0, int x1, int y);
static void port_span_slope(int16_t sector, uint8_t where, int x0, int x1, int y, port_plane_ctx ctx);
//-------------------------------------

//Function implementations

void port_planes_begin()
{
   //Clear planes
   for(int i = 0; i<128; i++)
   {
      port_plane_free(port_planes[i]);
      port_planes[i] = NULL;
   }
}

void port_planes_draw()
{
   //Render planes
   for(int i = 0;i<128;i++)
   {
      port_plane *pl = port_planes[i];
      while(pl!=NULL)
      {
         if(pl->min>pl->max)
         {
            pl = pl->next;
            continue;
         }

         //Parallax
         if((port_map->sectors[pl->sector].flags&RVR_PORT_SECTOR_PARALLAX_FLOOR&&pl->where==1)||
            (port_map->sectors[pl->sector].flags&RVR_PORT_SECTOR_PARALLAX_CEILING&&pl->where==0))
         {
            port_plane_sky(pl);

            pl = pl->next;
            continue;
         }

         //Slope
         if((port_map->sectors[pl->sector].slope_floor!=0&&pl->where==1)||
            (port_map->sectors[pl->sector].slope_ceiling!=0&&pl->where==0))
         {
            port_plane_slope(pl);

            pl = pl->next;
            continue;
         }

         //Flat
         port_plane_flat(pl);

         pl = pl->next;
      }
   
   }
}

void port_plane_add(int16_t sector, uint8_t where, int x, int y0, int y1)
{
   x+=1;

   //TODO: check how this hash actually performs (probably poorly...)
   int hash = (sector*7+where*3)&127;

   port_plane *pl = port_planes[hash];
   while(pl!=NULL)
   {
      //port_planes need to be in the same sector..
      if(sector!=pl->sector)
         goto next;
      //... and the same texture to be valid for concatination
      if(where!=pl->where)
         goto next;

      //Additionally the spans collumn needs to be either empty...
      if(pl->start[x]!=UINT16_MAX)
      {
         //... or directly adjacent to each other vertically, in that case
         //Concat planes vertically
         if(pl->start[x]-1==y1)
         {
            pl->start[x] = (uint16_t)y0;
            return;
         }
         if(pl->end[x]+1==y0)
         {
            pl->end[x] = (uint16_t)y1;
            return;
         }

         goto next;
      }

      break;
next:
      pl = pl->next;
   }

   if(pl==NULL)
   {
      pl = port_plane_new();
      pl->next= port_planes[hash];
      port_planes[hash] = pl;

      pl->min = RvR_xres();
      pl->max = -1;
      pl->sector = sector;
      pl->where = where;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(pl->start,255,sizeof(pl->start));
   }

   if(x<pl->min)
      pl->min = x;
   if(x>pl->max)
      pl->max = x;

   pl->end[x] = (uint16_t)y1;
   pl->start[x] = (uint16_t)y0;
}

static port_plane *port_plane_new()
{
   if(port_plane_pool==NULL)
   {
      port_plane *p = RvR_malloc(sizeof(*p)*8,"RvR_port plane pool");
      memset(p,0,sizeof(*p)*8);

      for(int i = 0;i<7;i++)
         p[i].next = &p[i+1];
      port_plane_pool = p;
   }

   port_plane *p = port_plane_pool;
   port_plane_pool = p->next;
   p->next = NULL;

   return p;
}

void port_plane_free(port_plane *pl)
{
   if(pl==NULL)
      return;

   //Find last
   port_plane *last = pl;
   while(last->next!=NULL)
      last = last->next;

   last->next = port_plane_pool;
   port_plane_pool = pl;
}

static void port_plane_sky(port_plane *pl)
{
   RvR_fix22 middle_row = (RvR_yres() / 2) + port_cam->shear;
   RvR_texture *texture = NULL;
   if(pl->where==1)
      texture = RvR_texture_get(port_map->sectors[pl->sector].floor_tex);
   else
      texture = RvR_texture_get(port_map->sectors[pl->sector].ceiling_tex);

   int skyw = 1 << RvR_log2(texture->width);
   int skyh = 1 << RvR_log2(texture->height);
   int mask = skyh - 1;

   RvR_fix16 angle_step = (skyw * 1024) / RvR_xres();
   RvR_fix16 tex_step = (1024 * skyh - 1) / RvR_yres();

   //TODO(Captain4LK): include fov in calculation
   RvR_fix22 angle = port_cam->dir*texture->width;
   angle += (pl->min - 1) * angle_step;

   for(int x = pl->min; x<pl->max + 1; x++)
   {
      //Sky is rendered fullbright, no lut needed
      uint8_t * restrict pix = &RvR_framebuffer()[(pl->start[x]) * RvR_xres() + x - 1];
      const uint8_t * restrict tex = &texture->data[((angle/1024) & (skyw - 1)) * skyh];
      const uint8_t * restrict col = RvR_shade_table(32);

      //Split in two parts: above and below horizon
      int middle = RvR_max(0, RvR_min(RvR_yres(), middle_row + RvR_yres() / 32));
      int tex_start = pl->start[x];
      int tex_end = middle;
      if(tex_end>pl->end[x])
         tex_end = pl->end[x];
      if(tex_start>tex_end)
         tex_end = tex_start;
      if(tex_start>middle)
         tex_end = tex_start - 1;
      int solid_end = pl->end[x];
      RvR_fix16 texture_coord = (RvR_yres() - middle + pl->start[x]) * tex_step;

      for(int y = tex_start; y<tex_end + 1; y++)
      {
         *pix = tex[texture_coord/1024];
         texture_coord += tex_step;
         pix += RvR_xres();

#if RVR_PORT_PIXELBYPIXEL
         if(RvR_key_pressed(RVR_PORT_PIXELKEY))
            RvR_render_present();
#endif
      }
      RvR_fix16 tex_coord = (RvR_yres()) * tex_step - 1;
      texture_coord = RvR_min(tex_coord, tex_coord - tex_step * (tex_end - middle));
      for(int y = tex_end + 1; y<solid_end + 1; y++)
      {
         *pix = col[tex[(texture_coord/1024) & mask]];
         texture_coord -= tex_step;
         pix += RvR_xres();

#if RVR_PORT_PIXELBYPIXEL
         if(RvR_key_pressed(RVR_PORT_PIXELKEY))
            RvR_render_present();
#endif
      }

      angle += angle_step;
   }
}

static void port_plane_flat(port_plane *pl)
{
   for(int x = pl->min;x<pl->max+2;x++)
   {
      RvR_fix22 s0 = pl->start[x-1];
      RvR_fix22 s1 = pl->start[x];
      RvR_fix22 e0 = pl->end[x-1];
      RvR_fix22 e1 = pl->end[x];

      //End spans top
      for(;s0<s1&&s0<=e0;s0++)
         port_span_flat(pl->sector,pl->where,port_span_start[s0],x-1,s0);

      //End spans bottom
      for(;e0>e1&&e0>=s0;e0--)
         port_span_flat(pl->sector,pl->where,port_span_start[e0],x-1,e0);

      //Start spans top
      for(;s1<s0&&s1<=e1;s1++)
         port_span_start[s1] = x-1;

      //Start spans bottom
      for(;e1>e0&&e1>=s1;e1--)
         port_span_start[e1] = x-1;
   }
}

static void port_plane_slope(port_plane *pl)
{
   RvR_port_slope slope;
   if(pl->where==0)
      RvR_port_slope_from_ceiling(port_map,pl->sector,&slope);
   else
      RvR_port_slope_from_floor(port_map,pl->sector,&slope);
   uint16_t wall_org = port_map->sectors[pl->sector].wall_first;
   RvR_fix22 x0 = port_map->walls[wall_org].x;
   RvR_fix22 y0 = port_map->walls[wall_org].y;
   x0&=~((1<<22)-1);
   y0&=~((1<<22)-1);
   //-x0
   //y0

   RvR_fix22 height = RvR_port_slope_height_at(&slope,port_cam->x,port_cam->y);

   RvR_fix22 px = -RvR_fix22_mul(port_cam->x-x0,RvR_fix22_cos(3072-port_cam->dir))+RvR_fix22_mul(port_cam->y-y0,RvR_fix22_sin(3072-port_cam->dir));
   RvR_fix22 pz = RvR_fix22_mul(port_cam->x-x0,RvR_fix22_sin(3072-port_cam->dir))+RvR_fix22_mul(port_cam->y-y0,RvR_fix22_cos(3072-port_cam->dir));
   RvR_fix22 py = RvR_port_slope_height_at(&slope,x0,y0)-port_cam->z;
   //RvR_fix22 px = 0;
   //RvR_fix22 py = 0;
   //RvR_fix22 pz = RvR_port_slope_height_at(&slope,px-x0,py-y0);

   RvR_fix22 mx = RvR_fix22_cos(2048+port_cam->dir);
   RvR_fix22 mz = RvR_fix22_sin(2048+port_cam->dir);
   RvR_fix22 my = RvR_port_slope_height_at(&slope,port_cam->x,port_cam->y+1024)-height;

   RvR_fix22 nx = RvR_fix22_sin(2048+port_cam->dir);
   RvR_fix22 nz = -RvR_fix22_cos(2048+port_cam->dir);
   RvR_fix22 ny = RvR_port_slope_height_at(&slope,port_cam->x+1024,port_cam->y)-height;

   RvR_fix22 u0 = RvR_fix22_mul(py,mz)-RvR_fix22_mul(pz,my);
   RvR_fix22 u1 = RvR_fix22_mul(pz,mx)-RvR_fix22_mul(px,mz);
   RvR_fix22 u2 = RvR_fix22_mul(px,my)-RvR_fix22_mul(py,mx);

   RvR_fix22 v0 = RvR_fix22_mul(py,nz)-RvR_fix22_mul(pz,ny);
   RvR_fix22 v1 = RvR_fix22_mul(pz,nx)-RvR_fix22_mul(px,nz);
   RvR_fix22 v2 = RvR_fix22_mul(px,ny)-RvR_fix22_mul(py,nx);

   RvR_fix22 z0 = RvR_fix22_mul(my,nz)-RvR_fix22_mul(mz,ny);
   RvR_fix22 z1 = RvR_fix22_mul(mz,nx)-RvR_fix22_mul(mx,nz);
   RvR_fix22 z2 = RvR_fix22_mul(mx,ny)-RvR_fix22_mul(my,nx);
   printf("%d %d %d\n",my,ny,py);
   
   port_plane_ctx ctx;
   ctx.u0 = u0;
   ctx.u1 = u1;
   ctx.u2 = u2*(RvR_xres()/2);
   ctx.v0 = v0;
   ctx.v1 = v1;
   ctx.v2 = v2*(RvR_xres()/2);
   ctx.z0 = z0;
   ctx.z1 = z1;
   ctx.z2 = z2*(RvR_xres()/2);
   printf("(%d %d %d) (%d %d %d) (%d %d %d)\n",u0,u1,u2,v0,v1,v2, z0, z1,z2);
   //printf("%d %d %d %d %d %d %d\n",slope.x, slope.y,slope.z,slope.d, z0, z1,z2);

   //printf("%d\n",RvR_port_slope_height_at(&slope,0,0));
   //printf("%d %d %d %d\n",slope.x, slope.y,slope.z,slope.d);
   
   //uint16_t wall_org = port_map->sectors[pl->sector].wall_first;

   //RvR_fix22 x_off = map->sectors[sector].x_off;
   //RvR_fix22 y_off = map->sectors[sector].y_off;

   //RvR_fix22 sorgx = RvR_fix22_mul(port_cam->x,RvR_fix22_cos(3072-port_cam->dir))-RvR_fix22_mul(port_cam->y,RvR_fix22_sin(3072-port_cam->dir));
   //RvR_fix22 sorgy = RvR_fix22_mul(port_cam->x,RvR_fix22_sin(3072-port_cam->dir))+RvR_fix22_mul(port_cam->y,RvR_fix22_cos(3072-port_cam->dir));
   //RvR_fix22 sorgz = RvR_port_slope_height_at(&slope,port_map->walls[wall_org].x,port_map->walls[wall_org].y)-port_cam->z;
   //RvR_fix22 

   for(int x = pl->min;x<pl->max+2;x++)
   {
      RvR_fix22 s0 = pl->start[x-1];
      RvR_fix22 s1 = pl->start[x];
      RvR_fix22 e0 = pl->end[x-1];
      RvR_fix22 e1 = pl->end[x];

      //End spans top
      for(;s0<s1&&s0<=e0;s0++)
         port_span_slope(pl->sector,pl->where,port_span_start[s0],x-1,s0,ctx);

      //End spans bottom
      for(;e0>e1&&e0>=s0;e0--)
         port_span_slope(pl->sector,pl->where,port_span_start[e0],x-1,e0,ctx);

      //Start spans top
      for(;s1<s0&&s1<=e1;s1++)
         port_span_start[s1] = x-1;

      //Start spans bottom
      for(;e1>e0&&e1>=s1;e1--)
         port_span_start[e1] = x-1;
   }
}

static void port_span_flat(int16_t sector, uint8_t where, int x0, int x1, int y)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   //TODO: investigate this further
   //Happens when a floor plane gets drawn at the horizon (should this happen?)
   if(y==RvR_yres()/2)
      return;

   RvR_texture *texture = NULL;
   if(where==0)
      texture = RvR_texture_get(port_map->sectors[sector].ceiling_tex);
   else if(where==1)
      texture = RvR_texture_get(port_map->sectors[sector].floor_tex);

   if(texture==NULL)
      return;

   int8_t shade_off = 0;
   RvR_fix22 height = 0;
   if(where==0)
   {
      height = port_map->sectors[sector].ceiling;
      shade_off = port_map->sectors[sector].shade_ceiling;
   }
   else if(where==1)
   {
      height = port_map->sectors[sector].floor;
      shade_off = port_map->sectors[sector].shade_floor;
   }

   RvR_fix22 view_sin = RvR_fix22_sin(port_cam->dir);
   RvR_fix22 view_cos = RvR_fix22_cos(port_cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(port_cam->fov/2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres()*fovx,RvR_xres()*1024);

   RvR_fix22 span_height = (height-port_cam->z);
   RvR_fix22 fdy = RvR_abs(RvR_yres()/2-y)*1024;

   int64_t slope = RvR_abs((span_height*(INT64_C(1)<<30))/RvR_non_zero(fdy));
   RvR_fix22 depth = RvR_fix22_div((RvR_yres()/2)*span_height,RvR_non_zero(RvR_fix22_mul(fovy,fdy)));
   depth = RvR_abs(depth);

   RvR_fix22 step_x = (RvR_fix22)(((int64_t)view_sin*slope)/(1<<18));
   RvR_fix22 step_y = (RvR_fix22)(((int64_t)view_cos*slope)/(1<<18));
   RvR_fix22 tx,ty;

   if((where==1&&port_map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_FLOOR)||
      (where==0&&port_map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_CEILING))
   {
      RvR_fix22 wx0 =port_map->walls[port_map->sectors[sector].wall_first].x;
      RvR_fix22 wy0 = port_map->walls[port_map->sectors[sector].wall_first].y;
      tx = -(port_cam->x-wx0)*1024*4-4*view_cos*depth+(x0-RvR_xres()/2)*step_x;;
      ty = (port_cam->y-wy0)*1024*4+4*view_sin*depth+(x0-RvR_xres()/2)*step_y;;
   }
   else
   {
      tx = -(port_cam->x&4095)*1024*4-4*view_cos*depth+(x0-RvR_xres()/2)*step_x;;
      ty = (port_cam->y&4095)*1024*4+4*view_sin*depth+(x0-RvR_xres()/2)*step_y;;
   }

   //Offset textures
   tx+=((int32_t)port_map->sectors[sector].x_off)*65536;
   ty+=((int32_t)port_map->sectors[sector].y_off)*65536;

   //Rotate textures
   if((where==1&&port_map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_FLOOR)||
      (where==0&&port_map->sectors[sector].flags&RVR_PORT_SECTOR_ALIGN_CEILING))
   {
      RvR_fix22 wx0 = port_map->walls[port_map->sectors[sector].wall_first].x;
      RvR_fix22 wy0 = port_map->walls[port_map->sectors[sector].wall_first].y;
      RvR_fix22 wx1 = port_map->walls[port_map->walls[port_map->sectors[sector].wall_first].p2].x;
      RvR_fix22 wy1 = port_map->walls[port_map->walls[port_map->sectors[sector].wall_first].p2].y;
      RvR_fix22 len = RvR_non_zero(RvR_fix22_sqrt(RvR_fix22_mul(wx0-wx1,wx0-wx1)+RvR_fix22_mul(wy0-wy1,wy0-wy1)));
      RvR_fix22 sp_cos = RvR_fix22_div(wx1-wx0,len);
      RvR_fix22 sp_sin = -RvR_fix22_div(wy1-wy0,len);
      RvR_fix22 tmp = tx;
      tx = RvR_fix22_mul(-sp_sin, tx) + RvR_fix22_mul(sp_cos, ty);
      ty = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, ty);
      tmp = step_x;
      step_x = RvR_fix22_mul(-sp_sin, step_x) + RvR_fix22_mul(sp_cos, step_y);
      step_y = RvR_fix22_mul(sp_cos, tmp) + RvR_fix22_mul(sp_sin, step_y);
   }

   //Rotate 90 degrees
   if((where==1&&port_map->sectors[sector].flags&RVR_PORT_SECTOR_ROT_FLOOR)||
      (where==0&&port_map->sectors[sector].flags&RVR_PORT_SECTOR_ROT_CEILING))
   {
   }

   RvR_fix22 x_log = RvR_log2(texture->width);
   RvR_fix22 y_log = RvR_log2(texture->height);
   RvR_fix22 x_and = (1<<x_log)-1;
   RvR_fix22 y_and = (1<<y_log)-1;
   y_log = RvR_max(0,16-y_log);
   x_and<<=(16-y_log);

   uint8_t * restrict pix = RvR_framebuffer()+y*RvR_xres()+x0;
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(depth>>12)+shade_off)));
   const uint8_t * restrict tex = texture->data;

   for(int x = x0;x<x1;x++)
   {
      uint8_t c = tex[((tx>>y_log)&x_and)+((ty>>16)&y_and)];
      *pix = col[c];
      tx+=step_x;
      ty+=step_y;
      pix++;

#if RVR_PORT_PIXELBYPIXEL
      if(RvR_key_pressed(RVR_PORT_PIXELKEY))
         RvR_render_present();
#endif
   }
}

static void port_span_slope(int16_t sector, uint8_t where, int x0, int x1, int y, port_plane_ctx ctx)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   RvR_texture *texture = NULL;
   if(where==0)
      texture = RvR_texture_get(port_map->sectors[sector].ceiling_tex);
   else if(where==1)
      texture = RvR_texture_get(port_map->sectors[sector].floor_tex);

   if(texture==NULL)
      return;

   RvR_fix22 step_x = ctx.u0;
   RvR_fix22 step_y = ctx.v0;
   RvR_fix22 tx = ctx.u2+ctx.u1*(RvR_yres()/2-y)+ctx.u0*(x0-RvR_xres()/2);
   RvR_fix22 ty = ctx.v2+ctx.v1*(RvR_yres()/2-y)+ctx.v0*(x0-RvR_xres()/2);

   RvR_fix22 z = ctx.z2+ctx.z1*(RvR_yres()/2-y)+ctx.z0*(x0-RvR_xres()/2);
   RvR_fix22 step_z = ctx.z0;
   //tx = RvR_fix22_div(tx,RvR_non_zero(z));
   //ty = RvR_fix22_div(ty,RvR_non_zero(z));
   //printf("%d %d %d %d\n",ty,step_y, z,step_z);
   //printf("%d\n",z);

   RvR_fix22 x_log = RvR_log2(texture->width);
   RvR_fix22 y_log = RvR_log2(texture->height);
   RvR_fix22 x_and = (1<<x_log)-1;
   RvR_fix22 y_and = (1<<y_log)-1;
   y_log = RvR_max(0,14-y_log);
   x_and<<=(14-y_log);

   uint8_t * restrict pix = RvR_framebuffer()+y*RvR_xres()+x0;
   const uint8_t * restrict col = RvR_shade_table((uint8_t)RvR_max(0,RvR_min(63,(0>>12)+0)));
   const uint8_t * restrict tex = texture->data;

   for(int x = x0;x<x1;x++)
   {
      RvR_fix22 u = RvR_fix22_div(tx*1024,RvR_non_zero(z));
      RvR_fix22 v = RvR_fix22_div(ty*1024,RvR_non_zero(z));
      uint8_t c = tex[((u>>y_log)&x_and)+((v>>14)&y_and)];
      *pix = col[c];
      tx+=step_x;
      ty+=step_y;
      z+=step_z;
      pix++;

#if RVR_PORT_PIXELBYPIXEL
      if(RvR_key_pressed(RVR_PORT_PIXELKEY))
         RvR_render_present();
#endif
   }
}
//-------------------------------------
