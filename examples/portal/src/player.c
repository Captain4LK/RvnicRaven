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
#include "gamestate.h"
#include "collision.h"
#include "book.h"
#include "bookcase.h"
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
   state->player.entity->pos[0] = 0;
   state->player.entity->pos[1] = 0;
   state->player.entity->pos[2] = 0;
   state->player.entity->dir = 0;
   state->player.entity->vel[0] = 0;
   state->player.entity->vel[1] = 0;
   state->player.entity->vel[2] = 0;
   state->player.entity->height = 1636;
   state->player.entity->radius = 256;
   state->player.entity->step_height = 256;
   state->player.entity->accel_mod = 1024;
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
   dirx = RvR_fix22_mul(dirx,player->entity->accel_mod);
   diry = RvR_fix22_mul(diry,player->entity->accel_mod);

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

   //Crouching
   if(RvR_key_pressed(config_crouch))
   {
      state->player.entity->pos[2]+=836;
      state->player.entity->height = 800;
      player->entity->accel_mod = 512;
   }
   else if(RvR_key_released(config_crouch))
   {
      state->player.entity->pos[2]-=836;
      state->player.entity->height = 1636;
      player->entity->accel_mod = 1024;
   }

   collision_move(state, state->player.entity);

   player->vis_off_vel += RvR_fix22_mul(player->entity->vel[2] - player->vis_off_vel, 450);
   RvR_fix22 dz = player->entity->pos[2] + (player->entity->height-200) - state->cam.z;
   //RvR_fix22 dz = player->entity->pos[2] + 1436 - state->cam.z;
   player->vis_off_vel += RvR_fix22_mul(dz * 64, 96);
   state->cam.z += player->vis_off_vel / 64;

   state->cam.dir = state->player.entity->dir;
   state->cam.x = state->player.entity->pos[0];
   state->cam.y = state->player.entity->pos[1];
   state->cam.sector = state->player.entity->sector;

   //Hover book
   int book_hover = 0;
   uint8_t book_case;
   uint8_t book_shelf;
   uint8_t book_slot;
   if((state->select.type==RVR_PORT_SWALL_BOT))
   {
      RvR_port_wall *wall = state->map->walls+state->select.as.wall;
      if(wall->tex_lower>(UINT16_MAX-BOOKCASE_COUNT)&&state->select.depth<2048)
      {
         int tx = state->select.tx;
         int ty = state->select.ty;
         book_case = -(wall->tex_lower-UINT16_MAX);

         if(tx<4||tx>=60)
         {
            book_hover = 0;
         }
         else if(ty>=8&&ty<=24)
         {
            book_shelf = 0;
            book_slot = (tx-4)/4;
            book_hover = 1;
         }
         else if(ty>=28&&ty<=48)
         {
            book_shelf = 1;
            book_slot = (tx-4)/4;
            book_hover = 1;
         }
         else if(ty>=52&&ty<=72)
         {
            book_shelf = 2;
            book_slot = (tx-4)/4;
            book_hover = 1;
         }
      }
   }

   if(book_hover)
   {
      //puts(book_get(bookcase_at(book_case,book_shelf,book_slot))->title);
   }
}
//-------------------------------------
