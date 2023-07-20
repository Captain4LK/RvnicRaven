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
#include "player.h"
#include "action.h"
#include "world_defs.h"
#include "area.h"
#include "entity.h"
#include "tile.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Player player;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_new(World *w, Area *a)
{
   player.e = entity_new(w);
   player.e->x = 0;
   player.e->y = 0;
   player.e->z = 15;
   player.e->speed = 128;
   player.e->ai_type = AI_PLAYER;
   entity_add(a, player.e);
   entity_grid_add(a, player.e);
}

void player_update()
{
   //Movement
   int dir = -1;
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
      const int dirs[8][2] =
      {
         { 1, 0 },
         { 0, 1 },
         { -1, 0 },
         { 0, -1 },

         //Diagonal
         { -1, 1 },
         { -1, -1 },
         { 1, -1 },
         { 1, 1 },
      };

      /*Entity *ent = map_ent_at(player.ent->x + dirs[dir][0], player.ent->y + dirs[dir][1]);
      if(ent!=NULL&&entity_alignment(player.ent, ent)<0)
      {
         //TODO attack
         puts("ATTACK");
      }
      else*/
      {
         action_set_move(player.e, dir);
      }
   }
}

int player_action(Area *a)
{
   if(player.e->action.id==ACTION_INVALID)
      return 0;

   action_do(a, player.e);
   //player.cam_auto = 1;

   return 1;
}

int player_pos_valid(Area *a, Entity *e, int x, int y, int z)
{
   uint32_t block = area_tile(a, x, y, z);
   uint32_t floor = area_tile(a, x, y, z + 1);

   if(!tile_has_wall(block)&&(tile_has_floor(floor)||tile_is_slope(floor)))
      return 1;
   return 0;
}
//-------------------------------------