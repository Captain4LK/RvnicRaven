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
#include "entity_defs.h"
#include "world.h"
#include "world_gen.h"
#include "area_gen.h"
#include "area_draw.h"
#include "area.h"
#include "action.h"
#include "camera.h"
#include "player.h"
#include "game.h"
#include "turn.h"
#include "item.h"
#include "region.h"
#include "util.h"
#include "spiral_path.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static World *world;
static Area *area;

static int redraw = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_update()
{
   if(RvR_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   player_update();

   if(player.e->action.id!=ACTION_INVALID)
   {
      if(action_do(area, player.e))
      {
         redraw = 1;
      }
   }

   int ox = player.e->x;
   int oy = player.e->y;
   int oz = player.e->z;
   if(player.e->action_points==0)
   {
      //Run entities
      turn_do(area);

      redraw = 1;
      player.e->action_points = player.e->speed;
   }

   if(redraw)
   {
      fov_player(area, player.e, ox, oy, oz);
   }

   //Camera
   //-------------------------------------
   if(RvR_key_pressed(RVR_KEY_C))
   {
      player.cam.z_cutoff = player.cam.z_cutoff?0:player.e->z;
      redraw = 1;
   }

   if(RvR_key_pressed(RVR_KEY_COMMA)&&!RvR_key_down(RVR_KEY_LSHIFT))
   {
      player.cam.rotation = (player.cam.rotation - 1) & 3;
      redraw = 1;
   }
   if(RvR_key_pressed(RVR_KEY_PERIOD)&&!RvR_key_down(RVR_KEY_LSHIFT))
   {
      player.cam.rotation = (player.cam.rotation + 1) & 3;
      redraw = 1;
   }

   if(player.cam.z_cutoff!=0)
   {
      if(player.cam.z_cutoff!=player.e->z)
         redraw = 1;
      player.cam.z_cutoff = player.e->z;
   }

   player.cam.z = player.e->z;

   switch(player.cam.rotation)
   {
   case 0: player.cam.x = player.e->x + 4; player.cam.y = player.e->y - 24; break;
   case 1: player.cam.x = player.e->y + 4; player.cam.y = -player.e->x + area->dimy * 32 - 1 - 24; break;
   case 2: player.cam.x = -player.e->x + area->dimx * 32 - 1 + 4; player.cam.y = -player.e->y + area->dimy * 32 - 1 - 24; break;
   case 3: player.cam.x = -player.e->y + area->dimy * 32 - 1 + 4; player.cam.y = player.e->x - 24; break;
   }
   //-------------------------------------

   if(RvR_key_pressed(RVR_KEY_P))
      area_save(world,area);
}

void game_draw()
{
   if(!redraw)
      return;
   redraw = 0;

   RvR_render_clear(0);
   area_draw_begin(world, area, &player.cam);

   //Draw entities
   Entity *e = area->entities;
   for(; e!=NULL; e = e->next)
      area_draw_sprite(e->tex, e->x, e->y, e->z);

   //Draw items
   Item *i = area->items;
   for(; i!=NULL; i = i->next)
      area_draw_sprite(i->tex, i->x, i->y, i->z);

   area_draw_end();
}

void game_init()
{
   //world = world_new("test", WORLD_SMALL);
   world = world_load("test");
   //area = area_load(world,0);
   //area = area_create(world, 0, 0, 1, 2, 1, 0);

   WorldGen_preset preset = {0};
   preset.lakes_deep = 8;
   preset.lakes_shallow = 0;
   preset.mountains_high = 16;
   preset.var_elevation = 8192;
   preset.var_temperature = 2048;
   preset.var_rainfall = 2048;
   //world_gen(world,3,&preset);

   area = area_gen(world,&preset,0,128,192,2,2,2,0);
   player_new(world, area);

   player.cam.x = 16;
   player.cam.y = 0;
   player.cam.z = 1;
   player.cam.z_cutoff = 0;

   unsigned dim = world_size_to_dim(world->size);
   /*unsigned dim = world_size_to_dim(world->size);
   for(int i = 0;i<dim*dim;i++)
   {
      Region *r = region_create(world,i%dim,i/dim);
      for(int j = 0;j<32*32;j++)
         r->tiles[j] = rand();
      r->tiles[0] = i;
      region_save(world,i%dim,i/dim);
   }*/

   //Region *r = region_get(world,0,0);
   //memset(r->tiles,0,sizeof(r->tiles));
   //r->tiles[0] = 1;
   //r->tiles[2] = 1;
   //region_save(world,0,0);

   /*for(int i = 0;i<dim*dim;i++)
   {
      Region *r = region_get(world,i%dim,i/dim);
      int check = 1;
      //for(int j = 0;j<32*32;j++)
      //{
         if(r->tiles[0]!=i)
         {
            check = 0;
            //break;
         }
      //}

      if(!check)
         printf("Mismatch in %d %d\n",i%dim,i/dim);
   }*/
}

void game_set()
{
   redraw = 1;
}
//-------------------------------------
