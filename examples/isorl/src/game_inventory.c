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
#include "state.h"
#include "player.h"
#include "game.h"
#include "draw.h"
#include "area_draw.h"
#include "game_inventory.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
int redraw = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_inventory_set()
{
   redraw = 1;
}

void game_inventory_init()
{
}

void game_inventory_update()
{
   if(RvR_key_pressed(RVR_KEY_ESCAPE))
   {
      state_set(STATE_GAME);
      return;
   }
}

void game_inventory_draw()
{
   if(!redraw)
      return;

   redraw = 0;

   RvR_render_clear(43);
   area_draw_begin(world, area, &player.cam);

   //Draw entities
   for(Entity *e = area->entities; e!=NULL; e = e->next)
      area_draw_entity(e, e->pos);

   //Draw items
   Item *i = area->items;
   for(; i!=NULL; i = i->next)
      area_draw_item(i, i->pos);

   area_draw_end();

   //Item list
   draw_fill_rectangle(32, 32, RvR_xres() - 64, RvR_yres() - 64, 1, 1);
   draw_line_horizontal(31, RvR_xres() - 32, 31, 42, 1);
   draw_line_horizontal(31, RvR_xres() - 32, RvR_yres() - 32, 42, 1);
   draw_line_vertical(31, 32, RvR_yres() - 33, 42, 1);
   draw_line_vertical(RvR_xres() - 32, 32, RvR_yres() - 33, 42, 1);
}
//-------------------------------------
