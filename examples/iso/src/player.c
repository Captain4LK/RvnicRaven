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
#include "player.h"
#include "action.h"
#include "color.h"
#include "world_defs.h"
#include "area.h"
#include "entity.h"
#include "tile.h"
#include "defs.h"
#include "log.h"
#include "item.h"
#include "draw.h"
#include "entity_documented.h"
#include "game.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Player player;

static Item **player_menu_items = NULL;
static int player_menu_select = 0;
//-------------------------------------

//Function prototypes
static int player_update_none();
static void player_draw_none();
static int player_update_pickup();
static void player_draw_pickup();
static int player_update_drop();
static void player_draw_drop();
//-------------------------------------

//Function implementations

void player_new(World *w, Area *a)
{
   Entity_documented ne = {0};
   uint64_t id = entity_doc_create(w);
   entity_doc_get(w, id, &ne);

   player.id = ne.id;
   ne.mx = 1;
   ne.my = 1;
   ne.ax = 0;
   ne.ay = 0;
   entity_doc_modify(w, ne.id, &ne);
}

void player_add(World *w, Area *a)
{
   player.menu_state = 0;
   player.e = entity_from_docent(w, a, player.id);
   player.e->speed = 128;
   player.e->ai_type = AI_PLAYER;
   entity_from_def(player.e, defs_get_entity("human"), 0);

   Entity *e = entity_new(w);
   e->speed = 128;
   e->pos = player.e->pos;
   entity_add(a, e);
   entity_grid_add(a, e);
   entity_from_def(e, defs_get_entity("human"), 1);

   Item *i = item_new(w);
   i->pos = player.e->pos;
   item_set_material(i,defs_get_material("aurum"));
   item_from_def(i,defs_get_item("sword"));
   item_add(a,i);
   item_grid_add(a,i);

   i = item_new(w);
   i->pos = player.e->pos;
   item_set_material(i,defs_get_material("skin"));
   item_from_def(i,defs_get_item("backpack"));
   item_add(a,i);
   item_grid_add(a,i);

   i = item_new(w);
   i->pos = player.e->pos;
   item_set_material(i,defs_get_material("bone"));
   item_from_def(i,defs_get_item("sword"));
   item_add(a,i);
   item_grid_add(a,i);
}

int player_update()
{
   switch(player.menu_state)
   {
   case 0: return player_update_none();
   case 1: return player_update_pickup();
   case 2: return player_update_drop();
   }

   return 0;
}

void player_menu_draw()
{
   switch(player.menu_state)
   {
   case 0: player_draw_none(); break;
   case 1: player_draw_pickup(); break;
   case 2: player_draw_drop(); break;
   }
}

static int player_update_none()
{
   if(player.e->action.id!=ACTION_INVALID)
   {}

   //Movement
   int8_t dir = -1;
   if(RvR_key_pressed(RVR_KEY_W)||(0&&RvR_key_pressed(RVR_KEY_UP))||RvR_key_pressed(RVR_KEY_NP7))
      dir = (3 + player.cam.rotation) & 3;
   if(RvR_key_pressed(RVR_KEY_S)||(0&&RvR_key_pressed(RVR_KEY_DOWN))||RvR_key_pressed(RVR_KEY_NP3))
      dir = (1 + player.cam.rotation) & 3;
   if(RvR_key_pressed(RVR_KEY_A)||(0&&RvR_key_pressed(RVR_KEY_LEFT))||RvR_key_pressed(RVR_KEY_NP1))
      dir = (2 + player.cam.rotation) & 3;
   if(RvR_key_pressed(RVR_KEY_D)||(0&&RvR_key_pressed(RVR_KEY_RIGHT))||RvR_key_pressed(RVR_KEY_NP9))
      dir = (0 + player.cam.rotation) & 3;
   if(RvR_key_pressed(RVR_KEY_NP6))
      dir = 4 + ((3 + player.cam.rotation) & 3);
   if(RvR_key_pressed(RVR_KEY_NP8))
      dir = 4 + ((2 + player.cam.rotation) & 3);
   if(RvR_key_pressed(RVR_KEY_NP4))
      dir = 4 + ((1 + player.cam.rotation) & 3);
   if(RvR_key_pressed(RVR_KEY_NP2))
      dir = 4 + ((player.cam.rotation) & 3);

   //Ascend/Descend
   if(RvR_key_pressed(RVR_KEY_COMMA)&&RvR_key_down(RVR_KEY_LSHIFT))
      action_set_descend(player.e);
   if(RvR_key_pressed(RVR_KEY_PERIOD)&&RvR_key_down(RVR_KEY_LSHIFT))
      action_set_ascend(player.e);

   if(dir!=-1)
   {
      //Check for attack target
      if(area_entity_at(area, point_add_dir(player.e->pos, dir), player.e)!=NULL)
         action_set_attack(player.e, dir);
      else
         action_set_move(player.e, dir);
   }

   //Pickup
   if(RvR_key_pressed(RVR_KEY_G))
   {
      //Create list of items on same tile as player
      RvR_array_length_set(player_menu_items,0);

      int gx = player.e->pos.x/8;
      int gy = player.e->pos.y/8;
      int gz = player.e->pos.z/8;

      Item *cur = area->item_grid[gz * area->dimy * 4 * area->dimx * 4 + gy * area->dimx * 4 + gx];
      for(; cur!=NULL; cur = cur->g_next)
      {
         if(cur->removed)
            continue;

         if(point_equal(cur->pos, player.e->pos))
            RvR_array_push(player_menu_items,cur);
      }

      if(RvR_array_length(player_menu_items)>=1)
      {
         player_menu_select = 0;
         player.menu_state = 1;
         return 1;
      }
   }

   //Drop
   if(RvR_key_pressed(RVR_KEY_D))
   {
      //Create list of items in dropable slots
      RvR_array_length_set(player_menu_items,0);

      for(int i = 0;i<player.e->body.part_count;i++)
      {
         if(player.e->body.parts[i].hp<=0)
            continue;

         for(int j = 0;j<player.e->body.parts[i].slot_count;j++)
         {
            if(player.e->body.parts[i].slots[j].type==ITEM_SLOT_GRASP)
            {
               Item *cur = player.e->body.parts[i].slots[j].it;
               for(;cur!=NULL;cur = cur->next)
                  RvR_array_push(player_menu_items,cur);
            }
         }
      }

      if(RvR_array_length(player_menu_items)>=1)
      {
         player_menu_select = 0;
         player.menu_state = 2;
         return 1;
      }
   }

   return 0;
}

static void player_draw_none()
{
   log_draw(0);

   //Draw status
   if(player.e->hunger>=2)
      RvR_render_string(1, 1, 1, "Hungry", 5);

   //Draw vital hp bars
   int dy = RvR_yres() - 74;
   for(int i = 0; i<player.e->body.part_count; i++)
   {
      Bodypart *bp = &player.e->body.parts[i];
      if(bp->def->tags & DEF_BODY_VITAL)
      {
         char tmp[128];
         snprintf(tmp, 128, "%5s: %3d/%3d", bp->def->name, bp->hp, bp->hp_max);
         RvR_render_string(1, dy, 1, tmp, 12);
         dy += 8;
      }
   }
}

static int player_update_pickup()
{
   int redraw = 0;

   if(RvR_key_pressed(RVR_KEY_UP))
   {
      player_menu_select--;
      if(player_menu_select<0)
         player_menu_select = 0;
      redraw = 1;
   }
   if(RvR_key_pressed(RVR_KEY_DOWN))
   {
      player_menu_select++;
      if(player_menu_select>=RvR_array_length(player_menu_items))
         player_menu_select = RvR_array_length(player_menu_items)-1;
      redraw = 1;
   }

   if(RvR_key_pressed(RVR_KEY_ENTER))
   {
      player.menu_state = 0;
      action_set_pickup(area,player.e, item_index_get(player_menu_items[player_menu_select]));
      return 1;
   }

   if(RvR_key_pressed(RVR_KEY_ESCAPE))
   {
      player.menu_state = 0;
      return 1;
   }

   return redraw;
}

static void player_draw_pickup()
{
   //Item list
   draw_fill_rectangle(32, 32, RvR_xres() - 64, RvR_yres() - 64, 1, 1);
   draw_line_horizontal(31, RvR_xres() - 32, 31, 42, 1);
   draw_line_horizontal(31, RvR_xres() - 32, RvR_yres() - 32, 42, 1);
   draw_line_vertical(31, 32, RvR_yres() - 33, 42, 1);
   draw_line_vertical(RvR_xres() - 32, 32, RvR_yres() - 33, 42, 1);

   for(int i = 0;i<RvR_array_length(player_menu_items);i++)
   {
      const char *str = item_name(player_menu_items[i]);
      if(i==player_menu_select)
      {
         RvR_render_string(42, 42+i*12, 1, str, color_white);
         RvR_render_string(36, 42+i*12, 1, ">", color_white);
      }
      else
      {
         RvR_render_string(42, 42+i*12, 1, str, color_light_gray);
      }
   }
}

static int player_update_drop()
{
   int redraw = 0;

   if(RvR_key_pressed(RVR_KEY_UP))
   {
      player_menu_select--;
      if(player_menu_select<0)
         player_menu_select = 0;
      redraw = 1;
   }
   if(RvR_key_pressed(RVR_KEY_DOWN))
   {
      player_menu_select++;
      if(player_menu_select>=RvR_array_length(player_menu_items))
         player_menu_select = RvR_array_length(player_menu_items)-1;
      redraw = 1;
   }

   if(RvR_key_pressed(RVR_KEY_ENTER))
   {
      player.menu_state = 0;
      action_set_drop(area,player.e, item_index_get(player_menu_items[player_menu_select]));
      return 1;
   }

   if(RvR_key_pressed(RVR_KEY_ESCAPE))
   {
      player.menu_state = 0;
      return 1;
   }

   return redraw;
}

static void player_draw_drop()
{
   //Item list
   draw_fill_rectangle(32, 32, RvR_xres() - 64, RvR_yres() - 64, 1, 1);
   draw_line_horizontal(31, RvR_xres() - 32, 31, 42, 1);
   draw_line_horizontal(31, RvR_xres() - 32, RvR_yres() - 32, 42, 1);
   draw_line_vertical(31, 32, RvR_yres() - 33, 42, 1);
   draw_line_vertical(RvR_xres() - 32, 32, RvR_yres() - 33, 42, 1);

   for(int i = 0;i<RvR_array_length(player_menu_items);i++)
   {
      const char *str = item_name(player_menu_items[i]);
      if(i==player_menu_select)
      {
         RvR_render_string(42, 42+i*12, 1, str, color_white);
         RvR_render_string(36, 42+i*12, 1, ">", color_white);
      }
      else
      {
         RvR_render_string(42, 42+i*12, 1, str, color_light_gray);
      }
   }
}
//-------------------------------------
