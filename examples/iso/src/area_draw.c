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
      int origin_y = (16*c->y-20*(z-c->z)-24)/16;
      int origin_x = -origin_y+c->x+c->y;
      int origin_z = z;
      int y = origin_y;

      //Would be infinite loop, but limited to prevent badness
      for(int i = 0;i<64;i++)
      {
         int min = RvR_max(-y+origin_x+origin_y,(20*(z-origin_z)+8*(y-origin_y)-RvR_yres())/8+origin_x);
         int max = RvR_min((RvR_xres()-16*(y-origin_y))/16+origin_x,(20*(z-origin_z)+8*(y-origin_y))/8+origin_x);
         //min = RvR_max(0,min);
         //max = RvR_min(a->dimx*32,max);

         if(min>max)
            break;

         for(int x = max-1;x>=min-3;x--)
         {
            if(area_tile(a,x,y,z)==0)
               continue;

            if(area_tile(a,x-1,y,z)!=0&&area_tile(a,x,y,z-1)!=0&&area_tile(a,x,y+1,z)!=0)
               continue;

            RvR_texture *tex = RvR_texture_get(1);
            RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y+12-cy);
            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
            tex = RvR_texture_get(0);
            RvR_render_texture(tex,x*16+y*16-cx,z*20-8*x+8*y-4-cy);
            if(RvR_key_pressed(RVR_KEY_SPACE))
               RvR_render_present();
         }
         y++;
      }
   }
}
//-------------------------------------
