/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "world_defs.h"
#include "world.h"
#include "game.h"
#include "region.h"
#include "game_map.h"
#include "area_draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int redraw = 0;
static Camera cam;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_map_update()
{
   if(RvR_key_down(RVR_KEY_LEFT))
      cam.x--;
   if(RvR_key_down(RVR_KEY_RIGHT))
      cam.x++;
   if(RvR_key_down(RVR_KEY_UP))
      cam.y--;
   if(RvR_key_down(RVR_KEY_DOWN))
      cam.y++;

   redraw = 1;
}

void game_map_draw()
{
   if(!redraw)
      return;

   RvR_render_clear(0);
   redraw = 0;

   cam.rotation = 0;
   unsigned dim = world_size_to_dim(world->size);

   int origin_y = (16 * cam.y) / 16;
   int origin_x = -origin_y + cam.x + cam.y;
   int y = origin_y;
   int cx = cam.x * 16 + cam.y * 16;
   int cy = - 8 * cam.x + 8 * cam.y;
   for(int i = 0; i<64; i++)
   {
      int min = RvR_max(-y + origin_x + origin_y, (8 * (y - origin_y) - RvR_yres()) / 8 + origin_x);
      int max = RvR_min((RvR_xres() - 16 * (y - origin_y)) / 16 + origin_x, (8 * (y - origin_y)) / 8 + origin_x);

      if(min>max)
         break;

      min = RvR_max(0, min - 1);
      if(cam.rotation==1||cam.rotation==3)
         max = RvR_min(dim * 32, max + 2);
      else
         max = RvR_min(dim * 32, max + 2);

      for(int x = max; x>=min; x--)
      {
         int tx = x;
         int ty = y;
         int txf = x;
         int tyr = y;
         switch(cam.rotation)
         {
         case 0: tx = x; ty = y; txf = tx - 1; tyr = ty + 1; break;
         case 1: tx = dim * 32 - 1 - y; ty = x; txf = tx - 1; tyr = ty - 1; break;
         case 2: tx = dim * 32 - 1 - x; ty = dim * 32 - 1 - y; txf = tx + 1; tyr = ty - 1; break;
         case 3: tx = y; ty = dim * 32 - 1 - x; txf = tx + 1; tyr = ty + 1; break;
         }

         if(x<0||y<0||x>=dim*32||y>=dim*32)
            continue;
         Region *r = region_get(world,x/32,y/32);
         if(r==NULL)
            continue;

         int32_t elevation = r->elevation[(y%32)*33+(x%32)]/512;
         //if(elevation<256)
            //continue;

         RvR_texture *tex = RvR_texture_get(2);
         int z = 0;
         RvR_render_texture(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - cy-elevation);

         //Outline
         //-------------------------------------
         int px = x * 16 + y * 16 - cx;
         int py = z * 20 - 8 * x + 8 * y - cy-elevation;

         if(x+1>=dim*32)
            continue;
         r = region_get(world,(x+1)/32,y/32);
         if(r==NULL)
            continue;
         if(elevation>r->elevation[(y%32)*33+((x+1)%32)]/512)
            RvR_render_line((px + 15) * 256 + 128, (py) * 256 + 128, (px + 31) * 256 + 128, (py + 8) * 256 + 128, 1);

         if(y-1<0)
            continue;
         r = region_get(world,x/32,(y-1)/32);
         if(r==NULL)
            continue;
         if(elevation>r->elevation[((y-1)%32)*33+(x%32)]/512)
            RvR_render_line((px + 1) * 256 + 128, (py + 7) * 256 + 128, (px + 17) * 256 + 128, (py - 1) * 256 + 128, 1);


         //-------------------------------------
      }
      y++;
   }
}

void game_map_init()
{
}

void game_map_set()
{
   redraw = 1;
   cam.x = 16;
   cam.y = 16;
}
//-------------------------------------
