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
#include "world_gen.h"
#include "area_gen.h"
#include "area.h"
#include "world.h"
#include "game.h"
#include "region.h"
#include "player.h"
#include "state.h"
#include "game_map.h"
#include "area_draw.h"
#include "entity_documented.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int redraw = 0;

//Function prototypes
//-------------------------------------

//Function implementations

void game_map_update()
{
   if(RvR_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   unsigned dim = world_size_to_dim(world->size);

   Entity_documented pe = {0};
   entity_doc_get(world, player.id, &pe);

   int moved = 0;
   if(pe.mx>1&&RvR_key_pressed(RVR_KEY_LEFT))
   {
      pe.mx--;
      moved = 1;
   }
   if(pe.mx<dim * 32 - 2&&RvR_key_pressed(RVR_KEY_RIGHT))
   {
      pe.mx++;
      moved = 1;
   }
   if(pe.my>1&&RvR_key_pressed(RVR_KEY_UP))
   {
      pe.my--;
      moved = 1;
   }
   if(pe.my<dim * 32 - 2&&RvR_key_pressed(RVR_KEY_DOWN))
   {
      pe.my++;
      moved = 1;
   }

   if(moved&&area!=NULL)
   {
      area_exit(world, area);
      area_free(world, area);
      area = NULL;
   }

   entity_doc_modify(world, player.id, &pe);

   if(RvR_key_pressed(RVR_KEY_D))
   {
      if(area==NULL)
      {
         area = area_gen(world, 1, pe.mx - 1, pe.my - 1, 3, 3, 2, 0);
         player_add(world, area);
      }

      state_set(STATE_GAME);
   }

   redraw = 1;
}

void game_map_draw()
{
   if(!redraw)
      return;

   RvR_render_clear(0);
   redraw = 0;

   Entity_documented pe = {0};
   entity_doc_get(world, player.id, &pe);

   Camera cam;
   cam.x = pe.mx + 4;
   cam.y = pe.my - 24;
   cam.rotation = 0;
   unsigned dim = world_size_to_dim(world->size);

   int32_t elevation_center = 0;
   Region *rc = region_get(world, pe.mx / 32, pe.my / 32);
   if(rc!=NULL)
   {
      elevation_center = rc->elevation[(pe.my % 32) * 33 + (pe.mx % 32)];
   }

   int origin_y = (16 * cam.y) / 16;
   int origin_x = -origin_y + cam.x + cam.y;
   int y = origin_y;
   int cx = cam.x * 16 + cam.y * 16;
   int cy = -8 * cam.x + 8 * cam.y;
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

         if(x>=dim * 32)
            continue;
         if(y>=dim * 32)
            continue;

         int32_t e0 = world_elevation(world, x, y) / 512;
         int32_t e1 = world_elevation(world, x + 1, y) / 512;
         int32_t e2 = world_elevation(world, x - 1, y) / 512;
         int32_t e3 = world_elevation(world, x, y + 1) / 512;
         int32_t e4 = world_elevation(world, x, y - 1) / 512;
         //if(elevation<256)
         //continue;

         RvR_texture *tex = RvR_texture_get(0);
         int z = 0;

         int collumn_height = RvR_min(8, RvR_max((e0 - e2) / 16, (e0 - e3) / 16));
         for(int h = collumn_height; h>=0; h--)
            RvR_render_texture(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - cy - e0 + elevation_center / 512 + h * 16);

         //Outline
         //-------------------------------------
         int px = x * 16 + y * 16 - cx;
         int py = z * 20 - 8 * x + 8 * y - cy - e0 + elevation_center / 512;

         if(e0>e1)
            RvR_render_line((px + 16) * 256 + 128, (py + 1) * 256 + 128, (px + 32) * 256 + 128, (py + 9) * 256 + 128, 1);

         if(e0>e4)
            RvR_render_line((px) * 256 + 128, (py + 8) * 256 + 128, (px + 16) * 256 + 128, (py) * 256 + 128, 1);
         //-------------------------------------

         if(x==pe.mx&&y==pe.my)
         {
            tex = RvR_texture_get(16384);
            RvR_render_texture(tex, x * 16 + y * 16 - cx, z * 20 - 8 * x + 8 * y - cy - e0 + elevation_center / 512 - 16);
         }
      }
      y++;
   }
}

void game_map_init()
{}

void game_map_set()
{
   redraw = 1;
}
//-------------------------------------
