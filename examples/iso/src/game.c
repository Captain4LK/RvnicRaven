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
#include "state.h"
#include "spiral_path.h"
#include "entity_documented.h"
#include "draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
World *world;
Area *area;

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
      int res = action_do(world,area,player.e);
      if(res)
         redraw = 1;
      if(res==ACTION_LEFT_MAP)
      {
         Entity_documented pe = {0};
         entity_doc_get(world,player.id,&pe);

         int mx = pe.mx-1;
         int my = pe.my-1;
         if(pe.mx>=area->mx+area->dimx) mx = pe.mx;
         else if(pe.mx<area->mx) mx = pe.mx-2;
         if(pe.my>=area->my+area->dimy) my = pe.my;
         else if(pe.my<area->my) my = pe.my-2;

         area_exit(world,area);
         area_free(world,area);
         area = area_gen(world,1,mx,my,3,3,2,0);

         player_add(world, area);
         state_set(STATE_GAME);
      }
   }

   int ox = player.e->x;
   int oy = player.e->y;
   int oz = player.e->z;
   if(player.e->action_points==0)
   {
      //Run entities
      turn_do(world,area);

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

   if(RvR_key_down(RVR_KEY_LSHIFT)&&RvR_key_pressed(RVR_KEY_T))
   {
      docent_from_entity(world,area,player.e);
      state_set(STATE_GAME_MAP);
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

   //draw_fill_rectangle(RvR_xres()/2-64,RvR_yres()/2-64,128,128,1,1);
   draw_fill_rectangle(RvR_xres()-256,RvR_yres()-96,256,96,1,1);
   RvR_render_string(RvR_xres()-256+1,RvR_yres()-96+1,1,"You ask me to explain why I am afraid of a draught\nof cool air; why I shiver more than others upon\nentering a cold room, and seem nauseated and\nrepelled when the chill of evening creeps through\nthe heat of a mild autumn day. There are those who\nsay I respond to cold as others do to a bad odour,\nand I am the last to deny the impression. What I\nwill do is to relate the most horrible circumstance\nI ever encountered, and leave it to you to judge\nwhether or not this forms a suitable explanation\nof my peculiarity.",8);
}

void game_init()
{
   world = world_new("test", WORLD_SMALL);
   //world = world_load("test");
   world->preset.lakes_deep = 8;
   world->preset.lakes_shallow = 0;
   world->preset.mountains_high = 16;
   world->preset.var_elevation = 8192;
   world->preset.var_temperature = 2048;
   world->preset.var_rainfall = 2048;
   world_gen(world,4);
   world_save(world);
   player_new(world,NULL);

   //puts("---------");
   //world_gen(world,5);
}

void game_set()
{
   redraw = 1;
}
//-------------------------------------
