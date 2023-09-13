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
#include "entity_documented.h"
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
   player.e = entity_from_docent(w, a, player.id);
   player.e->speed = 128;
   player.e->ai_type = AI_PLAYER;
   player.e->tex = 16384;

   Entity *e = entity_new(w);
   e->speed = 128;
   e->tex = 16384;
   e->x = player.e->x;
   e->y = player.e->y;
   e->z = player.e->z;
   entity_add(a, e);
   entity_grid_add(a, e);
}

void player_update()
{
   if(player.e->action.id!=ACTION_INVALID)
   {
      if(player.e->action.can_interrupt&&RvR_key_pressed(RVR_KEY_NP5))
         player.e->action.id = ACTION_INVALID;
      else
         return;
   }

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
      action_set_move(player.e, dir);
   }
}
//-------------------------------------
