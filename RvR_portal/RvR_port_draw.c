/*
RvnicRaven - portal drawing

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
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

   int16_t wall;
   int16_t sector;
}RvR_port_dwall;
//-------------------------------------

//Variables
static int16_t *sector_stack = NULL;
static RvR_port_dwall *dwalls = NULL;
//static int16_t sector_stack[RVR_PORT_SECTOR_STACK];
//-------------------------------------

//Function prototypes
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

         if(i==0||(w0-2)->p2!=i+map->sectors[sector].wall_first)
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

         dw.y0 = RvR_fix16_div(map->sectors[sector].floor-cam->z,RvR_non_zero(RvR_fix16_mul(fovy,dw.z0)));
         dw.y0 = RvR_yres()*32768-RvR_yres()*dw.y0;
         dw.y1 = RvR_fix16_div(map->sectors[sector].floor-cam->z,RvR_non_zero(RvR_fix16_mul(fovy,dw.z1)));
         dw.y1 = RvR_yres()*32768-RvR_yres()*dw.y1;

         RvR_array_push(dwalls,dw);
      }
   }
   //-------------------------------------

   printf("%d\n",RvR_array_length(dwalls));
}
//-------------------------------------
