/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "area_draw.h"
#include "area.h"
#include "camera.h"
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

void area_draw(const World *w, const Area *a, const Camera *c)
{
   int cx = c->x*16+c->y*16;
   int cy = c->z*20-8*c->x+8*c->y;

   for(int z = a->dimz*32-1;z>=0;z--)
   {
      //int y = c->y;
      int d = RvR_max(0,((-20*(z-c->z))/8+c->x-c->y)/2);
      int y = c->y+d;
      printf("%d\n",d);

      for(int i = 0;i<64;i++)
      {
         int min = RvR_max(-y+c->x-d+c->y,(20*(z-c->z)+8*(y-c->y)-RvR_yres())/8+c->x-d);
         int max = RvR_min((RvR_xres()-16*(y-c->y))/16+c->x-d,(20*(z-c->z)+8*(y-c->y))/8+c->x-d);
         //printf("%d %d\n",min,max);
         if(min>max)
         {
            printf("%d %d %d %d\n",y,min,max,i);
            break;
         }
         for(int x = max+1;x>=min;x--)
         {
            if(area_tile(a,x,y,z)==0)
               continue;

            if(area_tile(a,x-1,y,z)!=0&&area_tile(a,x,y,z-1)!=0&&area_tile(a,x,y+1,z)!=0)
               continue;

            RvR_texture *tex = RvR_texture_get(0);
            RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-cy);
            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
            tex = RvR_texture_get(1);
            RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);
            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
         }
         //printf("%d %d\n",min,max);
         y++;
      }
      /*for(int x = a->dimx*32-1;x>=0;x--)
      {
         int min_y = -x+c->x+c->y-1;
         int max_y = (RvR_xres()-16*(x-c->x))/16+c->y;
         for(int y = min_y;y<max_y;y++)
         {
            if(area_tile(a,x,y,z)==0)
               continue;

            if(area_tile(a,x-1,y,z)!=0&&area_tile(a,x,y,z-1)!=0&&area_tile(a,x,y+1,z)!=0)
               continue;

            RvR_texture *tex = RvR_texture_get(0);
            RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-cy);
            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
            tex = RvR_texture_get(1);
            RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);
            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
         }
      }*/
   }

   /*int height = RvR_yres()/20;
   int cx = c->x*16+c->y*16;
   int cy = c->z*20-8*c->x+8*c->y;

   int draws = 0;

   //int tz = 0;
   for(int tz = height+c->z-1;tz>=c->z;tz--)
   {
      int tx = c->x;
      int ty = c->y-1;
      for(int y = 0;y<=RvR_yres()/16;y++)
      {
         for(int x = 0;x<=RvR_xres()/32;x++)
         {
            int dx = tx+x;
            int dy = ty+x;

            if(area_tile(a,dx,dy,tz)==0)
               continue;

            RvR_texture *tex = RvR_texture_get(0);
            RvR_render_texture(tex,dx*16+dy*16-cx,tz*20-8*dx+8*dy-cy);
            tex = RvR_texture_get(1);
            RvR_render_texture(tex,dx*16+dy*16-cx,tz*20-8*dx+8*dy-4-cy);
         }

         tx--;

         for(int x = 0;x<=RvR_xres()/32;x++)
         {
            int dx = tx+x;
            int dy = ty+x;

            if(area_tile(a,dx,dy,tz)==0)
               continue;

            RvR_texture *tex = RvR_texture_get(0);
            RvR_render_texture(tex,dx*16+dy*16-cx,tz*20-8*dx+8*dy-cy);
            tex = RvR_texture_get(1);
            RvR_render_texture(tex,dx*16+dy*16-cx,tz*20-8*dx+8*dy-4-cy);
         }

         ty++;
      }
   }*/

   /*for(int tx = c->x;tx<cx;tx++)
   {
      int ty = cy+y_step-1;

      for(int i = 0;i<y_step;i++,ty--)
      {
         int tz = c->z+height;
         for(int j = 0;j<height;j++,tz--)
         {
            if(area_tile(a,tx,ty,tz)==0)
               continue;

            RvR_texture *tex = RvR_texture_get(0);
            RvR_render_texture(tex,tx*16+ty*16,tz*20+8*tx-8*ty);
            tex = RvR_texture_get(1);
            RvR_render_texture(tex,tx*16+ty*16,tz*20+8*tx-8*ty-4);
         }
      }

      y_step+=2;
   }*/

   /*int cx = RvR_xres()/32+c->x;
   int cy = RvR_xres()/32+c->y;
   int y_step = 2;
   int height = RvR_yres()/24;

   int draws = 0;

   for(;cx>=c->x;cx--,cy--)
   {
      int tx = cx;
      int ty = cy;

      for(int i = 0;i<y_step;i++,ty++)
      {
         int tz = c->z;
         for(int j = 0;j<height;j++,tz++)
         {
            RvR_texture *tex = RvR_texture_get(0);
            RvR_render_texture(tex,tx*16+ty*16,tz*24+8*tx-8*ty);
            draws++;
         }
      }

      y_step+=2;
   }
   printf("%d\n",draws);*/
}
//-------------------------------------
