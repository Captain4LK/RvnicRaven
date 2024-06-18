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
   state->player.entity->pos[0] = 13230;
   state->player.entity->pos[1] = -37173;
   state->player.entity->pos[2] = -8128;
   state->player.entity->dir = -10176;
   state->player.entity->vel[0] = 0;
   state->player.entity->vel[1] = 0;
   state->player.entity->vel[2] = 0;
   state->player.entity->height = 1636;
   state->player.entity->radius = 256;
   state->player.entity->step_height = 256;
}

void player_update(Gamestate *state)
{
   Player *player = &state->player;
   if(state==NULL)
      return;

   int x, y;
   RvR_mouse_relative_pos(&x, &y);

   //Mouse look: x-axis
   if(x!=0)
      state->player.entity->dir += (x * 128 * 2) / 32;

   //Shearing (fake looking up/down)
   //Drift back to 0
   if(!state->player.shearing&&state->cam.shear!=0)
   {
      int shear_step = ((RvR_yres() * 3) / (RvR_fps() * 4));
      state->cam.shear = (state->cam.shear>0)?(RvR_max(0, state->cam.shear - shear_step)):(RvR_min(0, state->cam.shear + shear_step));
   }

   //Enable freelook
   if(RvR_key_pressed(RVR_KEY_F))
      state->player.shearing = !state->player.shearing;

   //Mouse look: y-axis
   if(y!=0&&state->player.shearing)
      state->cam.shear = RvR_max(RvR_min(state->cam.shear - (y * 128) / 64, RvR_yres()/4), -RvR_yres()/4);

   RvR_fix22 dirx = RvR_fix22_cos(state->player.entity->dir);
   RvR_fix22 diry = RvR_fix22_sin(state->player.entity->dir);
   dirx *= 1;
   diry *= 1;

   if(RvR_key_down(config_move_forward))
   {
      state->player.entity->vel[0] += dirx;
      state->player.entity->vel[1] += diry;
   }
   else if(RvR_key_down(config_move_backward))
   {
      state->player.entity->vel[0] -= dirx;
      state->player.entity->vel[1] -= diry;
   }

   if(RvR_key_down(config_strafe_left))
   {
      state->player.entity->vel[0] += diry;
      state->player.entity->vel[1] -= dirx;
   }
   else if(RvR_key_down(config_strafe_right))
   {
      state->player.entity->vel[0] -= diry;
      state->player.entity->vel[1] += dirx;
   }

   if(RvR_key_pressed(config_jump)&&state->player.entity->on_ground)
   {
      state->player.entity->vel[2] = 11946;
   }
   state->player.entity->vel[2] -= 910;

   collision_move(state, state->player.entity);

   player->vis_off_vel += RvR_fix22_mul(player->entity->vel[2] - player->vis_off_vel, 450);
   RvR_fix22 dz = player->entity->pos[2] + 1436 - state->cam.z;
   player->vis_off_vel += RvR_fix22_mul(dz * 64, 96);
   state->cam.z += player->vis_off_vel / 64;

   state->cam.dir = state->player.entity->dir;
   state->cam.x = state->player.entity->pos[0];
   state->cam.y = state->player.entity->pos[1];
   state->cam.sector = state->player.entity->sector;

   //printf("%d %d %d %d\n", state->player.entity->pos[0], state->player.entity->pos[1], state->player.entity->pos[2],state->player.entity->dir);
}
//-------------------------------------
