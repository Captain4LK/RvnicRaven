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
#include "area_draw.h"
#include "area.h"
#include "camera.h"
#include "player.h"
#include "game.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static World *world;
static Area *area;
static Camera camera;

static int turns_to_do_frame;
static int turns_to_do_total;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_update()
{
   if(RvR_key_pressed(RVR_KEY_LEFT))
      camera.x--;
   if(RvR_key_pressed(RVR_KEY_RIGHT))
      camera.x++;
   if(RvR_key_pressed(RVR_KEY_UP))
      camera.y--;
   if(RvR_key_pressed(RVR_KEY_DOWN))
      camera.y++;

   if(RvR_key_pressed(RVR_KEY_Z))
      camera.z++;
   if(RvR_key_pressed(RVR_KEY_X))
      camera.z--;

   if(RvR_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   player_update();
   if(turns_to_do_frame==0&&player_action(area))
   {
      turns_to_do_total = player.e->turn_next;
      turns_to_do_frame = 4;
   }
   if(turns_to_do_frame)
   {
      turns_do(area,turns_to_do_total / 4+ ((turns_to_do_total % 4) >= turns_to_do_frame));
      turns_to_do_frame--;

      //for(int i = 0; i<map.width * map.height; i++)
         //map.discover[i] = map.discover[i]?1:0;
      //raycast_visibility();
   }
}

void game_draw()
{
   area_draw_begin(world,area,&camera);

   Entity *e = area->entities;
   for(;e!=NULL;e = e->next)
   {
      area_draw_sprite(e->tex,e->x,e->y,e->z);
   }

   area_draw_end();
}

void game_init()
{
   world = world_new("test",WORLD_SMALL);
   area = area_create(world,0,0,1,1,1,0);
   player_new(world,area);

   camera.x = 16;
   camera.y = 0;
   camera.z = 1;
}

void game_set()
{
}
//-------------------------------------
