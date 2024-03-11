/*
RvnicRaven - iso roguelike

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "point.h"
#include "log.h"
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
static void game_menu_none_update();
static void game_menu_none_draw();
//-------------------------------------

//Function implementations

void game_update()
{
   if(RvR_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   //if(turn_heap_peek_max!=player.e)
   //turns_do_until(
   if(turn_heap_peek_max()!=player.e)
      RvR_log_line("game_update", "warning: player not at start of turn queue\n");

   if(player_update())
      redraw = 1;

   Point old_pos = player.e->pos;
   if(player.e->action.id!=ACTION_INVALID)
   {
      //Pop heap
      turn_heap_max();

      int res = action_do(world, area, player.e);
      if(res)
         redraw = 1;
      if(res==ACTION_LEFT_MAP)
      {
         Entity_documented pe = {0};
         entity_doc_get(world, player.id, &pe);

         uint16_t mx = pe.mx - 1;
         uint16_t my = pe.my - 1;
         if(pe.mx>=area->mx + area->dimx) mx = pe.mx;
         else if(pe.mx<area->mx) mx = pe.mx - 2;
         if(pe.my>=area->my + area->dimy) my = pe.my;
         else if(pe.my<area->my) my = pe.my - 2;

         area_exit(world, area);
         area_free(world, area);
         area = area_gen(world, 1, mx, my, 3, 3, 2, 0);

         player_add(world, area);
         state_set(STATE_GAME);
      }

      if(player.e->action_points>0)
         turn_heap_push(player.e);
   }

   while(turn_heap_peek_max()!=player.e)
   {
      redraw = 1;
      turns_do_until(world, area, player.e);
      if(turn_heap_peek_max()==NULL)
         turn_start(world, area);
   }

   if(redraw)
   {
      fov_player(area, player.e, old_pos);
   }

   //Camera
   //-------------------------------------
   if(RvR_key_pressed(RVR_KEY_C))
   {
      player.cam.z_cutoff = player.cam.z_cutoff?0:player.e->pos.z;
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
      if(player.cam.z_cutoff!=player.e->pos.z)
         redraw = 1;
      player.cam.z_cutoff = player.e->pos.z;
   }

   if(RvR_key_down(RVR_KEY_LSHIFT)&&RvR_key_pressed(RVR_KEY_T))
   {
      docent_from_entity(world, area, player.e);
      state_set(STATE_GAME_MAP);
      return;
   }

   if(RvR_key_down(RVR_KEY_I))
   {
      state_set(STATE_GAME_INVENTORY);
      return;
   }

   player.cam.z = player.e->pos.z;

   switch(player.cam.rotation)
   {
   case 0: player.cam.x = player.e->pos.x + 2; player.cam.y = player.e->pos.y - 22; break;
   case 1: player.cam.x = player.e->pos.y + 2; player.cam.y = -player.e->pos.x + area->dimy * 32 - 1 - 22; break;
   case 2: player.cam.x = -player.e->pos.x + area->dimx * 32 - 1 + 2; player.cam.y = -player.e->pos.y + area->dimy * 32 - 1 - 22; break;
   case 3: player.cam.x = -player.e->pos.y + area->dimy * 32 - 1 + 2; player.cam.y = player.e->pos.x - 22; break;
   }
   //-------------------------------------

   if(RvR_key_pressed(RVR_KEY_P))
      area_save(world, area);
}

void game_draw()
{
   if(!redraw)
      return;

   redraw = 0;

   RvR_render_clear(43);
   area_draw_begin(world, area, &player.cam);

   //Draw entities
   Entity *e = area->entities;
   for(; e!=NULL; e = e->next)
   {
      if(e->removed)
         continue;
      area_draw_entity(e, e->pos);
   }

   //Draw items
   Item *it = area->items;
   for(; it!=NULL; it = it->next)
   {
      if(it->removed)
         continue;
      area_draw_item(it, it->pos);
   }

   area_draw_end();

   player_menu_draw();
}

void game_init()
{
   world = world_new("test", WORLD_SMALL);
   //world = world_load("test");
   world->preset.lakes_deep = 8;
   world->preset.lakes_shallow = 0;
   world->preset.mountains_high = 16;
   world->preset.var_elevation = 16384;
   world->preset.var_temperature = 2048;
   world->preset.var_rainfall = 2048;
   world_gen(world, 4);
   world_save(world);
   player_new(world, NULL);

   log_push("You ask me to explain why I am afraid of a draught of cool air; why I shiver more than others upon entering a cold room, and seem nauseated and repelled when the chill of evening creeps through the heat of a mild autumn day. There are those who say I respond to cold as others do to a bad odour, and I am the last to deny the impression. What I will do is to relate the most horrible circumstance I ever encountered, and leave it to you to judge whether or not this forms a suitable explanation of my peculiarity.");

   //puts("---------");
   //world_gen(world,5);
}

void game_set()
{
   fov_player(area, player.e, player.e->pos);
   redraw = 1;
   while(turn_heap_peek_max()!=player.e)
   {
      turns_do_until(world, area, player.e);
      if(turn_heap_peek_max()==NULL)
         turn_start(world, area);
   }
}
//-------------------------------------
