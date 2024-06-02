/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "player.h"
#include "entity.h"
#include "collision.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_create_new(Gamestate *state)
{
   if(state==NULL)
      return;

   state->player.entity = entity_new(state);
   state->player.entity->x = 0;
   state->player.entity->y = 0;
   state->player.entity->z = 0;
   state->player.entity->vx = 0;
   state->player.entity->vy = 0;
   state->player.entity->vz = 0;
   state->player.entity->col_height = 1792;
   state->player.entity->col_radius = 256;
}

void player_update(Gamestate *state)
{
   if(state==NULL)
      return;

   int x, y;
   RvR_mouse_relative_pos(&x, &y);

   //Mouse look: x-axis
   if(x!=0)
      state->player.entity->direction += (x * 128 * 2) / 32;

   RvR_fix22 dirx = RvR_fix22_cos(state->player.entity->direction);
   RvR_fix22 diry = RvR_fix22_sin(state->player.entity->direction);
   dirx*=1;
   diry*=1;

   if(RvR_key_down(config_move_forward))
   {
      state->player.entity->vx+=dirx;
      state->player.entity->vy+=diry;
   }
   else if(RvR_key_down(config_move_backward))
   {
      state->player.entity->vx-=dirx;
      state->player.entity->vy-=diry;
   }

   if(RvR_key_down(config_strafe_left))
   {
      state->player.entity->vx+=diry;
      state->player.entity->vy-=dirx;
   }
   else if(RvR_key_down(config_strafe_right))
   {
      state->player.entity->vx-=diry;
      state->player.entity->vy+=dirx;
   }

   collision_move(state,state->player.entity);

   state->cam.dir = state->player.entity->direction;
   state->cam.x = state->player.entity->x;
   state->cam.y = state->player.entity->y;
   state->cam.z = state->player.entity->z+1536;
   state->cam.sector = state->player.entity->sector;
}
//-------------------------------------
