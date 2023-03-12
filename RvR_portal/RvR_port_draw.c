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
   RvR_fix16 x0;
   RvR_fix16 y0;
   RvR_fix16 z0;
   RvR_fix16 x1;
   RvR_fix16 y1;
   RvR_fix16 z1;
   RvR_fix16 zback;

   int16_t wall;
   int16_t sector;
}RvR_port_dwall;
//-------------------------------------

//Variables
static int16_t *sector_stack = NULL;
static RvR_port_dwall *dwalls = NULL;
//-------------------------------------

//Function prototypes
static int dwall_comp(const void *a, const void *b);
static int dwall_order(const RvR_port_dwall *wa, const RvR_port_dwall *wb);
//-------------------------------------

//Function implementations

void RvR_port_draw(RvR_port_map *map, RvR_port_cam *cam)
{
   //Collect potentially visible walls
   //-------------------------------------
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres());
   RvR_fix16 sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 sin_fov = RvR_fix16_mul(sin,fovx);
   RvR_fix16 cos_fov = RvR_fix16_mul(cos,fovx);

   //TODO: bitmap for visited?
   RvR_array_length_set(dwalls,0);
   RvR_array_length_set(sector_stack,0);
   RvR_array_push(sector_stack,cam->sector);
   for(int i = 0;i<map->sector_count;i++)
      map->sectors[i].visited = 0;

   while(RvR_array_length(sector_stack)>0)
   {
      int16_t sector = sector_stack[RvR_array_length(sector_stack)-1];
      RvR_array_length_set(sector_stack,RvR_array_length(sector_stack)-1);
      map->sectors[sector].visited = 1;

      RvR_port_wall *w0 = &map->walls[map->sectors[sector].wall_first];
      RvR_port_wall *w1 = NULL;

      //Clipped an projected points
      RvR_fix16 tp0x = 0;
      RvR_fix16 tp0y = 0;
      RvR_fix16 tp1x = 0;
      RvR_fix16 tp1y = 0;
      for(int i = 0;i<map->sectors[sector].wall_count;i++,w0++)
      {
         w1 = &map->walls[w0->p2];

         RvR_fix16 x0 = w0->x-cam->x;
         RvR_fix16 y0 = w0->y-cam->y;
         RvR_fix16 x1 = w1->x-cam->x;
         RvR_fix16 y1 = w1->y-cam->y;
         RvR_port_dwall dw = {0};

         if(i==0||(w0-1)->p2!=i+map->sectors[sector].wall_first)
         {
            tp0x = RvR_fix16_mul(-x0,sin)+RvR_fix16_mul(y0,cos);
            tp0y = RvR_fix16_mul(x0,cos_fov)+RvR_fix16_mul(y0,sin_fov);
         }
         else
         {
            tp0x = tp1x;
            tp0y = tp1y;
         }
         tp1x = RvR_fix16_mul(-x1,sin)+RvR_fix16_mul(y1,cos);
         tp1y = RvR_fix16_mul(x1,cos_fov)+RvR_fix16_mul(y1,sin_fov);

         //Behind camera
         if(tp0y<-128&&tp1y<-128)
            continue;

         //Not facing camera --> winding
         if(RvR_fix16_mul(tp0x,tp1y)-RvR_fix16_mul(tp1x,tp0y)>0)
            continue;

         //Clipping
         //Left point in fov
         if(tp0x>=-tp0y)
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            dw.x0 = RvR_min(RvR_xres()*32768+RvR_fix16_div(tp0x*(RvR_xres()/2),tp0y),RvR_xres()*65536-1);
            dw.z0 = tp0y;
         }
         //Left point to the left of fov
         else
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            RvR_fix16 dx0 = tp1x-tp0x;
            RvR_fix16 dx1 = tp0x+tp0y;
            dw.x0 = 0;
            dw.z0 = RvR_fix16_div(RvR_fix16_mul(dx0,dx1),tp1y-tp0y+tp1x-tp0x)-tp0x;
         }

         //Right point in fov
         if(tp1x<=tp1y)
         {
            //Completely out of sight
            if(tp1x<-tp1y)
               continue;

            //TODO: tpy1-1?
            dw.x1 = RvR_min(RvR_xres()*32768+RvR_fix16_div(tp1x*(RvR_xres()/2),tp1y),RvR_xres()*65536-1);
            dw.z1 = tp1y;
         }
         else
         {
            //Completely out of sight
            if(tp0x>tp0y)
               continue;

            RvR_fix16 dx0 = tp1x-tp0x;
            RvR_fix16 dx1 = tp0y-tp0x;
            dw.x1 = RvR_xres()*65536-1;
            dw.z1 = tp0x-RvR_fix16_div(RvR_fix16_mul(dx0,dx1),tp1y-tp0y-tp1x+tp0x);
         }

         if(dw.x0>dw.x1)
            continue;

         //Add portal
         int16_t portal = w0->portal;
         if(portal>=0&&!map->sectors[portal].visited)
            RvR_array_push(sector_stack,portal);

         //Near clip
         if(dw.z0<16||dw.z1<16)
            continue;

         if(dw.z0>dw.z1)
            dw.zback = dw.z0;
         else
            dw.zback = dw.z1;

         dw.y0 = RvR_fix16_div(map->sectors[sector].floor-cam->z,RvR_non_zero(RvR_fix16_mul(fovy,dw.z0)));
         dw.y0 = RvR_yres()*32768-RvR_yres()*dw.y0;
         dw.y1 = RvR_fix16_div(map->sectors[sector].floor-cam->z,RvR_non_zero(RvR_fix16_mul(fovy,dw.z1)));
         dw.y1 = RvR_yres()*32768-RvR_yres()*dw.y1;
         dw.wall = map->sectors[sector].wall_first+i;
         dw.sector = sector;

         RvR_array_push(dwalls,dw);
      }
   }
   //-------------------------------------

   //Sort walls
   qsort(dwalls,RvR_array_length(dwalls),sizeof(*dwalls),dwall_comp);
   int len = RvR_array_length(dwalls);
   for(int i = 0;i<len;i++)
   {
      int swaps = 0;
      int j = i+1;
      while(j<len)
      {
         if(dwall_order(dwalls+i,dwalls+j)!=2)
         {
            j++;
         }
         else if(i+swaps>j)
         {
            //RvR_log("shit");
            break;
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
   int16_t *ytop = RvR_malloc(sizeof(*ytop)*RvR_xres(),"RvR_portal wall drawing top clip");
   int16_t *ybot = RvR_malloc(sizeof(*ybot)*RvR_xres(),"RvR_portal wall drawing bottom clip");
   memset(ytop,0,sizeof(*ytop)*RvR_xres());
   for(int i = 0;i<RvR_xres();i++)
      ybot[i] = RvR_yres()-1;
   for(int i = len-1;i>=0;i--)
   {
      RvR_port_dwall *wall = dwalls+i;
      int16_t sector = wall->sector;
      int16_t portal = map->walls[wall->wall].portal;

      if(portal<0)
      {
         int x0 = wall->x0>>16;
         int x1 = wall->x1>>16;
         int width = x1-x0;

         RvR_fix16 cy0 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),wall->z0);
         RvR_fix16 cy1 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),wall->z1);
         cy0 = RvR_yres()*32768-cy0;
         cy1 = RvR_yres()*32768-cy1;
         RvR_fix16 step_cy = RvR_fix16_div(cy1-cy0,wall->x1-wall->x0);
         //RvR_fix16 step_cy = (cy1-cy0)/RvR_non_zero(width);
         RvR_fix16 cy = cy0;

         RvR_fix16 fy0 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].floor-cam->z),wall->z0);
         RvR_fix16 fy1 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].floor-cam->z),wall->z1);
         fy0 = RvR_yres()*32768-fy0;
         fy1 = RvR_yres()*32768-fy1;
         RvR_fix16 step_fy = RvR_fix16_div(fy1-fy0,wall->x1-wall->x0);
         //RvR_fix16 step_fy = (fy1-fy0)/RvR_non_zero(width);
         RvR_fix16 fy = fy0;

         for(int x = x0;x<=x1;x++)
         {
            int wy =  ytop[x];
            uint8_t * restrict pix = RvR_framebuffer()+(wy*RvR_xres()+x);

            //Ceiling
            int y_to = RvR_min(cy>>16,ybot[x]);
            if(y_to>wy)
            {
               wy = y_to;
               pix = RvR_framebuffer()+(wy*RvR_xres()+x);
            }

            //Wall
            y_to = RvR_min((fy>>16)-1,ybot[x]);
            for(;wy<y_to;wy++)
            {
               *pix = i+24;
               pix+=RvR_xres();
            }

            cy+=step_cy;
            fy+=step_fy;
            ytop[x] = RvR_yres();
            ybot[x] = 0;

            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
         }
      }
      //Portal
      else
      {
         int x0 = wall->x0>>16;
         int x1 = wall->x1>>16;
         int width = x1-x0;

         RvR_fix16 cy0 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),wall->z0);
         RvR_fix16 cy1 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].ceiling-cam->z),wall->z1);
         cy0 = RvR_yres()*32768-cy0;
         cy1 = RvR_yres()*32768-cy1;
         RvR_fix16 step_cy = RvR_fix16_div(cy1-cy0,wall->x1-wall->x0);
         RvR_fix16 cy = cy0;

         RvR_fix16 cph0 = RvR_max(0,RvR_fix16_div(RvR_yres()*(map->sectors[sector].ceiling-map->sectors[portal].ceiling),wall->z0));
         RvR_fix16 cph1 = RvR_max(0,RvR_fix16_div(RvR_yres()*(map->sectors[sector].ceiling-map->sectors[portal].ceiling),wall->z1));
         RvR_fix16 step_cph = RvR_fix16_div(cph1-cph0,wall->x1-wall->x0);
         RvR_fix16 cph = cph0;

         RvR_fix16 fy0 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].floor-cam->z),wall->z0);
         RvR_fix16 fy1 = RvR_fix16_div(RvR_yres()*(map->sectors[sector].floor-cam->z),wall->z1);
         fy0 = RvR_yres()*32768-fy0;
         fy1 = RvR_yres()*32768-fy1;
         RvR_fix16 step_fy = RvR_fix16_div(fy1-fy0,wall->x1-wall->x0);
         RvR_fix16 fy = fy0;

         RvR_fix16 fph0 = RvR_max(0,RvR_fix16_div(RvR_yres()*(map->sectors[portal].floor-map->sectors[sector].floor),wall->z0));
         RvR_fix16 fph1 = RvR_max(0,RvR_fix16_div(RvR_yres()*(map->sectors[portal].floor-map->sectors[sector].floor),wall->z1));
         RvR_fix16 step_fph = RvR_fix16_div(fph1-fph0,wall->x1-wall->x0);
         RvR_fix16 fph = fph0;

         for(int x = x0;x<=x1;x++)
         {
            int wy =  ytop[x];
            uint8_t * restrict pix = RvR_framebuffer()+(wy*RvR_xres()+x);

            //Draw ceiling until ceiling wall
            int y_to = RvR_min(cy>>16,ybot[x]);
            if(y_to>wy)
            {
               //port_plane_add(wall->sector,0,x,wy,y_to);
               wy = y_to;
               pix = &RvR_framebuffer()[wy*RvR_xres()+x];
            }

            //if(RvR_core_key_down(RVR_KEY_SPACE))
               //RvR_core_render_present();

            //Draw ceiling wall
            y_to = RvR_min((cy+cph)>>16,ybot[x]);
            for(;wy<y_to;wy++)
            {
               *pix = 32;
               pix+=RvR_xres();
            }

            ytop[x] = wy;
            wy = RvR_max(wy,(fy-fph)>>16);
            pix = &RvR_framebuffer()[wy*RvR_xres()+x];

            //if(RvR_core_key_down(RVR_KEY_SPACE))
               //RvR_core_render_present();

            //Draw floor wall
            y_to = RvR_min((fy>>16)-1,ybot[x]);
            for(;wy<y_to;wy++)
            {
               *pix = 32;
               pix+=RvR_xres();
            }

            //if(RvR_core_key_down(RVR_KEY_SPACE))
               //RvR_core_render_present();

            //Draw floor
            /*y_to = RvR_min(RVR_YRES-1,port_ymax[x]);
            if(y_to>wy)
               port_plane_add(wall->sector,1,x,wy,y_to);

            if(RvR_core_key_down(RVR_KEY_SPACE))
               RvR_core_render_present();*/

            ybot[x] = RvR_min((fy-fph)>>16,ybot[x]);

            cy+=step_cy;
            cph+=step_cph;
            fy+=step_fy;
            fph+=step_fph;
         }
      }
   }

   RvR_free(ytop);
   RvR_free(ybot);
}

static int dwall_comp(const void *a, const void *b)
{
   const RvR_port_dwall *wa = a;
   const RvR_port_dwall *wb = b;

   return wb->zback-wa->zback;
}

static int dwall_order(const RvR_port_dwall *wa, const RvR_port_dwall *wb)
{
   int64_t x00 = wa->x0;
   int64_t y00 = wa->z0;
   int64_t x01 = wa->x1;
   int64_t y01 = wa->z1;
   int64_t x10 = wb->x0;
   int64_t y10 = wb->z0;
   int64_t x11 = wb->x1;
   int64_t y11 = wb->z1;

   //(x00/y00) is origin, all calculations centered arround it
   //t0 is relation of (b,p0) to wall a
   //t1 is relation of (b,p1) to wall a
   //See RvR_port_sector_inside for more in depth explanation
   int64_t x = x01-x00;
   int64_t y = y01-y00;
   //int32_t t0 = ((x10-x00)*y-(y10-y00)*x)/1024;
   //int32_t t1 = ((x11-x00)*y-(y11-y00)*x)/1024;
   int64_t t0 = ((x10-x00)*y-(y10-y00)*x);
   int64_t t1 = ((x11-x00)*y-(y11-y00)*x);

   //wa completely behind wb
   if(RvR_max(y00,y01)<RvR_min(y10,y11))
      return 1;

   //no overlap
   if(x00>x11||x01<x10)
      return 0;

   //if(RvR_min(y00>y11||y01<y10)
   //{
      //if(y00>y10)
         //return 1;
      //return 2;
   //}

   //walls on the same line (identicall or adjacent)
   if(t0==0&&t1==0)
      return 0;

   //(b,p0) on extension of wall a (shared corner, etc)
   //Set t0 = t1 to trigger RvR_sign_equal check (and for the return !RvR_sign_equal to be correct)
   if(t0==0)
   {
      //puts("CASE A");
      t0 = t1;
      //return 0;
   }

   //(b,p1) on extension of wall a
   //Set t0 = t1 to trigger RvR_sign_equal check
   if(t1==0)
   {
      //puts("CASE B");
      t1 = t0;
      //return 0;
   }

   //Wall either completely to the left or to the right of other wall
   if(RvR_sign_equal(t0,t1))
   {
      //Compare with player position relative to wall a
      //if wall b and the player share the same relation, wall a needs to be drawn first
      t1 = ((0-x00)*y-(0-y00)*x);
      //printf("%d %d\n",t0,t1);
      return (!RvR_sign_equal(t0,t1))+1;
   }

   //Extension of wall a intersects with wall b
   //--> check wall b instead
   //(x10/y10) is origin, all calculations centered arround it
   x = x11-x10;
   y = y11-y10;
   t0 = ((x00-x10)*y-(y00-y10)*x);
   t1 = ((x01-x10)*y-(y01-y10)*x);

   //(a,p0) on extension of wall b
   if(t0==0)
      t0 = t1;

   //(a,p1) on extension of wall b
   if(t1==0)
      t1 = t0;

   //Wall either completely to the left or to the right of other wall
   if(RvR_sign_equal(t0,t1))
   {
      //Compare with player position relative to wall b
      //if wall a and the player share the same relation, wall b needs to be drawn first
      t1 = ((0-x10)*y-(0-y10)*x);
      return RvR_sign_equal(t0,t1)+1;
   }

   //Invalid case (walls are intersecting), expect rendering glitches
   return 0;
}
//-------------------------------------
