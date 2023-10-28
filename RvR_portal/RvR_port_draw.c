/*
RvnicRaven - portal drawing

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   RvR_fix22 x0;
   RvR_fix22 y0;
   RvR_fix22 z0;
   RvR_fix22 x1;
   RvR_fix22 y1;
   RvR_fix22 z1;
   RvR_fix22 zfront;

   RvR_fix22 u0;
   RvR_fix22 u1;

   int16_t wall;
   int16_t sector;
}RvR_port_dwall;

typedef struct port_plane port_plane;
struct port_plane
{
   int32_t min;
   int32_t max;
   int16_t sector;
   uint8_t where; //0 --> ceiling; 1 --> floor
   uint16_t start[RVR_XRES_MAX+2];
   uint16_t end[RVR_XRES_MAX+2];

   port_plane *next;
};
//-------------------------------------

//Variables
static int16_t *sector_stack = NULL;
static RvR_port_dwall *dwalls = NULL;

static port_plane *port_planes[128] = {0};
static port_plane *port_plane_pool = NULL;

static RvR_fix22 port_span_start[RVR_YRES_MAX] = {0};
static RvR_fix22 port_ytop[RVR_XRES_MAX] = {0};
static RvR_fix22 port_ybot[RVR_XRES_MAX] = {0};
//-------------------------------------

//Function prototypes
static int dwall_comp(const void *a, const void *b);
static int dwall_can_front(const RvR_port_dwall *wa, const RvR_port_dwall *wb);

static void port_span_draw(RvR_port_map *map, RvR_port_cam *cam, int16_t sector, uint8_t where, int x0, int x1, int y);

static void port_plane_add(int16_t sector, uint8_t where, int x, int y0, int y1);
static port_plane *port_plane_new();
static void port_plane_free(port_plane *pl);
//-------------------------------------

//Function implementations

void RvR_port_draw(RvR_port_map *map, RvR_port_cam *cam)
{
   //Init
   //-------------------------------------
   for(int i = 0;i<128;i++)
   {
      port_plane_free(port_planes[i]);
      port_planes[i] = NULL;
   }

   RvR_fix22 fovx = RvR_fix22_tan(cam->fov/2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres()*fovx*2,RvR_xres()*1024);
   RvR_fix22 sin = RvR_fix22_sin(cam->dir);
   RvR_fix22 cos = RvR_fix22_cos(cam->dir);
   RvR_fix22 sin_fov = RvR_fix22_mul(sin,fovx);
   RvR_fix22 cos_fov = RvR_fix22_mul(cos,fovx);
   //-------------------------------------

   //Collect potentially visible walls
   //-------------------------------------

   //TODO: bitmap for visited?
   RvR_array_length_set(dwalls,0);
   RvR_array_length_set(sector_stack,0);
   for(int i = 0;i<map->sector_count;i++)
      map->sectors[i].visited = 0;
   RvR_array_push(sector_stack,cam->sector);
   map->sectors[cam->sector].visited = 1;
   while(RvR_array_length(sector_stack)>0)
   {
      int16_t sector = sector_stack[RvR_array_length(sector_stack)-1];
      RvR_array_length_set(sector_stack,RvR_array_length(sector_stack)-1);

      RvR_port_wall *w0 = &map->walls[map->sectors[sector].wall_first];
      RvR_port_wall *w1 = NULL;

      //Clipped and projected points
      RvR_fix22 tp0x = 0;
      RvR_fix22 tp0y = 0;
      RvR_fix22 tp1x = 0;
      RvR_fix22 tp1y = 0;
      for(int i = 0;i<map->sectors[sector].wall_count;i++,w0++)
      {
         RvR_texture *tex = RvR_texture_get(8);
         w1 = &map->walls[w0->p2];

         RvR_fix22 x0 = w0->x-cam->x;
         RvR_fix22 y0 = w0->y-cam->y;
         RvR_fix22 x1 = w1->x-cam->x;
         RvR_fix22 y1 = w1->y-cam->y;
         RvR_port_dwall dw = {0};

         if(i==0||(w0-1)->p2!=i+map->sectors[sector].wall_first)
         {
            tp0x = RvR_fix22_mul(-x0,sin)+RvR_fix22_mul(y0,cos);
            tp0y = RvR_fix22_mul(x0,cos_fov)+RvR_fix22_mul(y0,sin_fov);
         }
         else
         {
            tp0x = tp1x;
            tp0y = tp1y;
         }
         tp1x = RvR_fix22_mul(-x1,sin)+RvR_fix22_mul(y1,cos);
         tp1y = RvR_fix22_mul(x1,cos_fov)+RvR_fix22_mul(y1,sin_fov);

         //Behind camera
         if(tp0y<-128&&tp1y<-128)
            continue;

         //Not facing camera --> winding
         if(RvR_fix22_mul(tp0x,tp1y)-RvR_fix22_mul(tp1x,tp0y)>0)
            continue;

         dw.u0 = 0;
         //dw.u1 = tex->width*65536-1;
         dw.u1 = RvR_fix22_sqrt(RvR_fix22_mul(w1->x-w0->x,w1->x-w0->x)+RvR_fix22_mul(w1->y-w0->y,w1->y-w0->y))*tex->width-1;

         //Clipping
         //Left point in fov
         if(tp0x>=-tp0y)
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            dw.x0 = RvR_min(RvR_xres()*512+RvR_fix22_div(tp0x*(RvR_xres()/2),RvR_non_zero(tp0y)),RvR_xres()*1024);
            dw.z0 = tp0y;
         }
         //Left point to the left of fov
         else
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            dw.x0 = 0;
            RvR_fix22 dx0 = tp1x-tp0x;
            RvR_fix22 dx1 = tp0x+tp0y;
            dw.u0 = dw.u0 + RvR_fix22_div(RvR_fix22_mul(-tp0x-tp0y,dw.u1-dw.u0),RvR_non_zero(tp1x-tp0x+tp1y-tp0y));
            dw.z0 = RvR_fix22_div(RvR_fix22_mul(dx0,dx1),RvR_non_zero(tp1y-tp0y+tp1x-tp0x))-tp0x;
         }

         //Right point in fov
         if(tp1x<=tp1y)
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            //TODO: tpy1-1?
            dw.x1 = RvR_min(RvR_xres()*512+RvR_fix22_div(tp1x*(RvR_xres()/2),RvR_non_zero(tp1y)),RvR_xres()*1024);
            dw.z1 = tp1y;
         }
         else
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            RvR_fix22 dx0 = tp1x-tp0x;
            RvR_fix22 dx1 = tp0y-tp0x;
            dw.x1 = RvR_xres()*1024;
            dw.z1 = tp0x-RvR_fix22_div(RvR_fix22_mul(dx0,dx1),RvR_non_zero(tp1y-tp0y-tp1x+tp0x));
            dw.u1 = RvR_fix22_div(RvR_fix22_mul(dx1,dw.u1),RvR_non_zero(-tp1y+tp0y+tp1x-tp0x));
         }

         if(dw.x0>dw.x1)
            continue;

         //Add portal
         int16_t portal = w0->portal;
         if(portal>=0&&!map->sectors[portal].visited)
         {
            RvR_array_push(sector_stack,portal);
            map->sectors[portal].visited = 1;
         }

         //Near clip
         //TODO: this value probably needs adjusting (or a proper near clip perhaps?)
         if(dw.z0<128||dw.z1<128)
            continue;

         if(dw.z0<dw.z1)
            dw.zfront = dw.z0;
         else
            dw.zfront = dw.z1;

         dw.y0 = RvR_fix22_div(map->sectors[sector].floor-cam->z,RvR_non_zero(RvR_fix22_mul(fovy,dw.z0)));
         dw.y0 = RvR_fix22_mul(RvR_yres()*1024,512-dw.y0);
         dw.y1 = RvR_fix22_div(map->sectors[sector].floor-cam->z,RvR_non_zero(RvR_fix22_mul(fovy,dw.z1)));
         dw.y1 = RvR_fix22_mul(RvR_yres()*1024,512-dw.y1);
         dw.wall = map->sectors[sector].wall_first+i;
         dw.sector = sector;

         RvR_array_push(dwalls,dw);
      }
   }
   //-------------------------------------

   //Sort walls
   //Basically Newell's algorithm
   qsort(dwalls,RvR_array_length(dwalls),sizeof(*dwalls),dwall_comp);
   int len = RvR_array_length(dwalls);
   for(int i = 0;i<len;i++)
   {
      int swaps = 0;
      int j = i+1;
      while(j<len)
      {
         if(dwall_can_front(dwalls+i,dwalls+j))
         {
            j++;
         }
         else if(i+swaps>j)
         {
            //This case usually happens when walls intersect
            //Here we would split the wall, 
            //but since intersecting walls aren't supported we just pretend nothing happended
            j++;
         }
         else
         {
            RvR_port_dwall tmp = dwalls[j];
            for(int w = j;w>i;w--)
               dwalls[w] = dwalls[w-1];
            dwalls[i] = tmp;
            j = i+1;
            swaps++;
         }
      }
   }

   //And finally... Draw!
   memset(port_ytop,0,sizeof(*port_ytop)*RvR_xres());
   for(int i = 0;i<RvR_xres();i++)
      port_ybot[i] = RvR_yres()-1;
   for(int i = 0;i<len;i++)
   {
      RvR_port_dwall *wall = dwalls+i;
      int16_t sector = wall->sector;
      int16_t portal = map->walls[wall->wall].portal;

      //No portal
      if(portal<0)
      {
         RvR_fix22 cy0 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 cy1 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         cy0 = RvR_yres()*512-cy0;
         cy1 = RvR_yres()*512-cy1;
         RvR_fix22 step_cy = RvR_fix22_div(cy1-cy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 cy = cy0;

         RvR_fix22 fy0 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].floor-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 fy1 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].floor-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         fy0 = RvR_yres()*512-fy0;
         fy1 = RvR_yres()*512-fy1;
         RvR_fix22 step_fy = RvR_fix22_div(fy1-fy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 fy = fy0;

         RvR_fix22 denom = RvR_fix22_mul(wall->z1,wall->z0);
         RvR_fix22 num_step_z = 4*(wall->z0-wall->z1);
         RvR_fix22 num_z = 4*wall->z1;

         //Shrinks u coordinates, results in more
         //accurate interpolation of texture coordinates
         RvR_texture *texture = RvR_texture_get(8);
         int over = wall->u0/(texture->width*1024);
         wall->u0-=over*texture->width*1024;
         wall->u1-=over*texture->width*1024;

         RvR_fix22 num_step_u = RvR_fix22_mul(wall->z0,wall->u1)-RvR_fix22_mul(wall->z1,wall->u0);
         RvR_fix22 num_u = RvR_fix22_mul(wall->u0,wall->z1);

         //Adjust for fractional part
         int x0 = (wall->x0+1023)/1024;
         int x1 = (wall->x1+1023)/1024;
         RvR_fix22 xfrac = wall->x0-x0*1024;
         cy-=RvR_fix22_mul(xfrac,step_cy);
         fy-=RvR_fix22_mul(xfrac,step_fy);
         num_z-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(wall->x1-wall->x0));
         num_u-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u),RvR_non_zero(wall->x1-wall->x0));

         for(int x = x0;x<x1;x++)
         {
            int y0 = (cy+1023)/1024;
            int y1 = fy/1024;

            int wy = port_ytop[x];
            uint8_t * restrict pix = RvR_framebuffer()+(wy*RvR_xres()+x);

            //Ceiling
            int y_to = RvR_min(y0,port_ybot[x]);
            if(y_to>wy)
            {
               port_plane_add(wall->sector,0,x,wy,y_to-1);
               wy = y_to;
               pix = RvR_framebuffer()+(wy*RvR_xres()+x);
            }

            //Wall
            y_to = RvR_min((fy/1024)-1,port_ybot[x]);
            RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 nu = num_u+RvR_fix22_div(num_step_u*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz/4));
            RvR_fix22 u = (4*nu)/RvR_non_zero(nz);
            RvR_fix22 height = map->sectors[wall->sector].ceiling-cam->z;
            RvR_fix22 coord_step_scaled = (4*fovy*depth)/RvR_yres();
            RvR_fix22 texture_coord_scaled = height*4096+(wy-RvR_yres()/2+1)*coord_step_scaled;
            RvR_texture *texture = RvR_texture_get(8);
            const uint8_t * restrict tex = &texture->data[(((uint32_t)u)%texture->width)*texture->height];
            const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(depth>>9))));
            RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;
            for(;wy<=y_to;wy++)
            {
               *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
               pix+=RvR_xres();
               texture_coord_scaled+=coord_step_scaled;
            }

            //Floor
            y_to = RvR_min(RvR_yres()-1,port_ybot[x]);
            if(y_to>wy)
               port_plane_add(wall->sector,1,x,wy,y_to);

            port_ytop[x] = RvR_yres();
            port_ybot[x] = 0;

            cy+=step_cy;
            fy+=step_fy;

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
         }
      }
      //Portal
      else
      {
         RvR_fix22 cy0 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 cy1 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         cy0 = RvR_yres()*512-cy0;
         cy1 = RvR_yres()*512-cy1;
         RvR_fix22 step_cy = RvR_fix22_div(cy1-cy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 cy = cy0;

         RvR_fix22 cph0 = RvR_max(0,RvR_fix22_div(RvR_yres()*(map->sectors[sector].ceiling-map->sectors[portal].ceiling),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy))));
         RvR_fix22 cph1 = RvR_max(0,RvR_fix22_div(RvR_yres()*(map->sectors[sector].ceiling-map->sectors[portal].ceiling),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy))));
         RvR_fix22 step_cph = RvR_fix22_div(cph1-cph0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 cph = cph0;

         RvR_fix22 fy0 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].floor-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy)));
         RvR_fix22 fy1 = RvR_fix22_div(RvR_yres()*(map->sectors[sector].floor-cam->z),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy)));
         fy0 = RvR_yres()*512-fy0;
         fy1 = RvR_yres()*512-fy1;
         RvR_fix22 step_fy = RvR_fix22_div(fy1-fy0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 fy = fy0;

         RvR_fix22 fph0 = RvR_max(0,RvR_fix22_div(RvR_yres()*(map->sectors[portal].floor-map->sectors[sector].floor),RvR_non_zero(RvR_fix22_mul(wall->z0,fovy))));
         RvR_fix22 fph1 = RvR_max(0,RvR_fix22_div(RvR_yres()*(map->sectors[portal].floor-map->sectors[sector].floor),RvR_non_zero(RvR_fix22_mul(wall->z1,fovy))));
         RvR_fix22 step_fph = RvR_fix22_div(fph1-fph0,RvR_non_zero(wall->x1-wall->x0));
         RvR_fix22 fph = fph0;

         RvR_fix22 denom = RvR_fix22_mul(wall->z1,wall->z0);
         RvR_fix22 num_step_z = 4*(wall->z0-wall->z1);
         RvR_fix22 num_z = 4*wall->z1;

         //Shrinks u coordinates, results in more
         //accurate interpolation of texture coordinates
         RvR_texture *texture = RvR_texture_get(8);
         int over = wall->u0/(texture->width*1024);
         wall->u0-=over*texture->width*1024;
         wall->u1-=over*texture->width*1024;

         RvR_fix22 num_step_u = RvR_fix22_mul(wall->z0,wall->u1)-RvR_fix22_mul(wall->z1,wall->u0);
         RvR_fix22 num_u = RvR_fix22_mul(wall->u0,wall->z1);

         //Adjust for fractional part
         int x0 = (wall->x0+1023)/1024;
         int x1 = (wall->x1+1023)/1024;
         RvR_fix22 xfrac = wall->x0-x0*1024;
         cy-=RvR_fix22_mul(xfrac,step_cy);
         cph-=RvR_fix22_mul(xfrac,step_cph);
         fy-=RvR_fix22_mul(xfrac,step_fy);
         fph-=RvR_fix22_mul(xfrac,step_fph);
         num_z-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_z),RvR_non_zero(wall->x1-wall->x0));
         num_u-=RvR_fix22_div(RvR_fix22_mul(xfrac,num_step_u),RvR_non_zero(wall->x1-wall->x0));

         for(int x = x0;x<x1;x++)
         {
            int y0 = (cy+1023)/1024;
            int y1 = fy/1024;

            RvR_fix22 nz = num_z+RvR_fix22_div(num_step_z*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 nu = num_u+RvR_fix22_div(num_step_u*(x-x0),RvR_non_zero(wall->x1-wall->x0));
            RvR_fix22 depth = RvR_fix22_div(denom,RvR_non_zero(nz/4));
            RvR_fix22 u = (4*nu)/RvR_non_zero(nz);
            int wy =  port_ytop[x];
            uint8_t * restrict pix = RvR_framebuffer()+(wy*RvR_xres()+x);
            const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(depth>>14))));

            //Draw ceiling until ceiling wall
            int y_to = RvR_min(y0,port_ybot[x]);
            if(y_to>wy)
            {
               port_plane_add(wall->sector,0,x,wy,y_to-1);
               wy = y_to;
               pix = &RvR_framebuffer()[wy*RvR_xres()+x];
            }

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();

            //Draw ceiling wall
            RvR_fix22 height = map->sectors[portal].ceiling-cam->z;
            RvR_fix22 coord_step_scaled = (4*fovy*depth)/RvR_yres();
            RvR_fix22 texture_coord_scaled = height*4096+(wy-RvR_yres()/2+1)*coord_step_scaled;
            RvR_texture *texture = RvR_texture_get(8);
            const uint8_t * restrict tex = &texture->data[(((uint32_t)u)%texture->width)*texture->height];
            RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;
            y_to = RvR_min((cy+cph)/1024,port_ybot[x]);
            for(;wy<=y_to;wy++)
            {
               *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
               pix+=RvR_xres();

               texture_coord_scaled+=coord_step_scaled;
            }

            port_ytop[x] = wy;
            wy = RvR_max(wy,(fy-fph)/1024);
            pix = &RvR_framebuffer()[wy*RvR_xres()+x];

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();

            //Draw floor wall
            height = map->sectors[portal].floor-cam->z;
            coord_step_scaled = (4*fovy*depth)/RvR_yres();
            texture_coord_scaled = height*4096+(wy-RvR_yres()/2+1)*coord_step_scaled;
            texture = RvR_texture_get(8);
            tex = &texture->data[(((uint32_t)u)%texture->width)*texture->height];
            y_and = (1<<RvR_log2(texture->height))-1;
            y_to = RvR_min((fy/1024)-1,port_ybot[x]);
            for(;wy<=y_to;wy++)
            {
               *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
               pix+=RvR_xres();

               texture_coord_scaled+=coord_step_scaled;
            }

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();

            //Draw floor
            y_to = RvR_min(RvR_yres()-1,port_ybot[x]);
            if(y_to>wy)
               port_plane_add(wall->sector,1,x,wy,y_to);

            port_ybot[x] = RvR_min((fy-fph)/1024,port_ybot[x]);

            cy+=step_cy;
            cph+=step_cph;
            fy+=step_fy;
            fph+=step_fph;
         }
      }
   }

   //Render floor planes
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

         for(int x = pl->min;x<pl->max+2;x++)
         {
            RvR_fix22 s0 = pl->start[x-1];
            RvR_fix22 s1 = pl->start[x];
            RvR_fix22 e0 = pl->end[x-1];
            RvR_fix22 e1 = pl->end[x];

            //End spans top
            for(;s0<s1&&s0<=e0;s0++)
               port_span_draw(map,cam,pl->sector,pl->where,port_span_start[s0],x-1,s0);

            //End spans bottom
            for(;e0>e1&&e0>=s0;e0--)
               port_span_draw(map,cam,pl->sector,pl->where,port_span_start[e0],x-1,e0);

            //Start spans top
            for(;s1<s0&&s1<=e1;s1++)
               port_span_start[s1] = x-1;

            //Start spans bottom
            for(;e1>e0&&e1>=s1;e1--)
               port_span_start[e1] = x-1;
         }

         pl = pl->next;
      }
   
   }
}

static int dwall_comp(const void *a, const void *b)
{
   const RvR_port_dwall *wa = a;
   const RvR_port_dwall *wb = b;

   return wa->zfront-wb->zfront;
}

//Calculates wether wa can be drawn in front of wb
static int dwall_can_front(const RvR_port_dwall *wa, const RvR_port_dwall *wb)
{
   int64_t x00 = wa->x0;
   int64_t z00 = wa->z0;
   int64_t x01 = wa->x1;
   int64_t z01 = wa->z1;
   int64_t x10 = wb->x0;
   int64_t z10 = wb->z0;
   int64_t x11 = wb->x1;
   int64_t z11 = wb->z1;

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

   //no overlap
   if(x00>=x11||x01<=x10)
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

static void port_span_draw(RvR_port_map *map, RvR_port_cam *cam, int16_t sector, uint8_t where, int x0, int x1, int y)
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
      texture = RvR_texture_get(map->sectors[sector].ceiling_tex);
   else if(where==1)
      texture = RvR_texture_get(map->sectors[sector].floor_tex);

   if(texture==NULL)
      return;

   RvR_fix22 height = 0;
   if(where==0)
      height = map->sectors[sector].ceiling;
   else if(where==1)
      height = map->sectors[sector].floor;

   RvR_fix22 view_sin = RvR_fix22_sin(cam->dir);
   RvR_fix22 view_cos = RvR_fix22_cos(cam->dir);
   RvR_fix22 fovx = RvR_fix22_tan(cam->fov/2);
   RvR_fix22 fovy = RvR_fix22_div(RvR_yres()*fovx*2,RvR_xres()*1024);

   RvR_fix22 dy = RvR_yres()/2-y;
   RvR_fix22 depth = RvR_fix22_div(RvR_abs(cam->z-height),RvR_non_zero(fovy));
   depth = RvR_fix22_div(depth*RvR_yres(),RvR_non_zero(RvR_abs(dy)*1024)); //TODO

   RvR_fix22 x_log = RvR_log2(texture->width);
   RvR_fix22 y_log = RvR_log2(texture->height);
   RvR_fix22 step_x = ((view_sin*(cam->z-height))/RvR_non_zero(dy));
   RvR_fix22 step_y = ((view_cos*(cam->z-height))/RvR_non_zero(dy));
   RvR_fix22 tx = (cam->x&1023)*1024+(view_cos*depth)+(x0-RvR_xres()/2)*step_x;
   RvR_fix22 ty = -(cam->y&1023)*1024-(view_sin*depth)+(x0-RvR_xres()/2)*step_y;
   RvR_fix22 x_and = (1<<x_log)-1;
   RvR_fix22 y_and = (1<<y_log)-1;

   uint8_t * restrict pix = RvR_framebuffer()+y*RvR_xres()+x0;
   const uint8_t * restrict col = RvR_shade_table(RvR_max(0,RvR_min(63,(depth>>15))));
   const uint8_t * restrict tex = texture->data;

   for(int x = x0;x<x1;x++)
   {
      uint8_t c = tex[(((tx>>14)&x_and)<<y_log)+((ty>>14)&y_and)];
      *pix = col[c];
      tx+=step_x;
      ty+=step_y;
      pix++;
      //if(RvR_key_pressed(RVR_KEY_SPACE))
         //RvR_render_present();
   }
}

static void port_plane_add(int16_t sector, uint8_t where, int x, int y0, int y1)
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
            pl->start[x] = y0;
            return;
         }
         if(pl->end[x]+1==y0)
         {
            pl->end[x] = y1;
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

   pl->end[x] = y1;
   pl->start[x] = y0;
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

static void port_plane_free(port_plane *pl)
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
//-------------------------------------
