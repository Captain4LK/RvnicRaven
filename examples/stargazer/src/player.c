/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "game.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
#include "collision.h"
#include "grid.h"
#include "state.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Player player = {0};

static int shearing = 0;
static uint32_t move_start_tick = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_create_new()
{
   player.entity->col_height = 65536;
   player.entity->col_radius = 16384;
   player.entity->cards_size = 71;
   player.entity->cards = RvR_malloc(sizeof(*player.entity->cards) * player.entity->cards_size, "AI:player state");

   for(int i = 0; i<player.entity->cards_size; i++)
      player.entity->cards[i].type = CARD_NONE;

   //Starting inventory
   //Health
   card_health(&player.entity->cards[0], 10);
   card_health(&player.entity->cards[1], 10);

   player.cam.x = player.entity->x;
   player.cam.y = player.entity->y;
   player.cam.z = player.entity->z + player.entity->vis_zoff + CAMERA_COLL_HEIGHT_BELOW;
   player.cam.dir = player.entity->direction;
   player.cam.fov = 16384;
   player.cam.shear = player.shear = 0;

   //if(player.entity!=NULL)
   //entity_free(player.entity);
   //player.entity = entity_new();
}

void player_update()
{
   //Input
   int x, y;
   RvR_mouse_relative_pos(&x, &y);
   RvR_fix16 dirx = RvR_fix16_cos(player.entity->direction);
   RvR_fix16 diry = RvR_fix16_sin(player.entity->direction);

   //Forward/Backward movement
   if(RvR_key_down(config_move_forward))
   {
      player.entity->vx += dirx;
      player.entity->vy += diry;
   }
   else if(RvR_key_down(config_move_backward))
   {
      player.entity->vx -= dirx;
      player.entity->vy -= diry;
   }

   //Strafing
   if(RvR_key_down(config_strafe_left))
   {
      player.entity->vx += diry;
      player.entity->vy -= dirx;
   }
   else if(RvR_key_down(config_strafe_right))
   {
      player.entity->vx -= diry;
      player.entity->vy += dirx;
   }

   if(RvR_key_pressed(config_use))
      grid_entity_use(player.entity);

   //Mouse look: x-axis
   if(x!=0)
      player.entity->direction += (x * 128 * 4) / 32;

   //Shearing (fake looking up/down)
   //Drift back to 0
   if(!shearing&&player.shear!=0)
      player.shear = (player.shear>0)?(RvR_max(0, player.shear - CAMERA_SHEAR_STEP_FRAME)):(RvR_min(0, player.shear + CAMERA_SHEAR_STEP_FRAME));
   //Enable freelook
   if(RvR_key_pressed(RVR_KEY_F))
      shearing = !shearing;
   //Mouse look: y-axis
   if(y!=0&&shearing)
      player.shear = RvR_max(RvR_min(player.shear - (y * 128) / 64, CAMERA_SHEAR_MAX_PIXELS), -CAMERA_SHEAR_MAX_PIXELS);

   //Only for testing --> flying basically
   //if(RvR_core_key_down(RVR_KEY_PGDN))
   //player.vertical_speed = -step*100;
   //else if(RvR_core_key_down(RVR_KEY_PGUP))
   //player.vertical_speed = step*100;

   //Jumping (hacky but works)
   if(RvR_key_pressed(config_jump)&&player.entity->on_ground)
   {
      //sound_play(SOUND_PLAYER_JUMP,ai_index_get(player.entity),255);
      player.entity->vz = 764586;
   }
   player.entity->vz -= 58254;

   //Inventory
   if(RvR_key_pressed(config_inventory))
      state_set(STATE_GAME_INVENTORY);

   //Weapon switching
   /*if(RvR_core_key_pressed(RVR_KEY_1))
      player_weapon_switch(0);
   if(RvR_core_key_pressed(RVR_KEY_2))
      player_weapon_switch(1);
   if(RvR_core_key_pressed(RVR_KEY_3))
      player_weapon_switch(2);
   if(RvR_core_key_pressed(RVR_KEY_4))
      player_weapon_switch(3);
   player_weapon_rotate(RvR_core_mouse_wheel_scroll());

   if(player_weapon_ammo(player.weapon)==0)
      player_weapon_rotate(-1);*/

   //Collision
   RvR_fix16 floor_height = 0;
   RvR_fix16 ceiling_height = 0;
   collision_move(player.entity, &floor_height, &ceiling_height);

   //Reset verticall speed if ceiling was hit
   if(player.entity->z + player.entity->col_height>=ceiling_height)
      player.entity->vz = 0;

   //Enable jumping if on ground
   if(player.entity->z==floor_height)
   {
      if(!player.entity->on_ground)
         player.entity->vis_zoff = (player.entity->vz) / 48;
      player.entity->on_ground = 1;
      player.entity->vz = 0;
   }
   else
   {
      player.entity->on_ground = 0;
   }

   player.vis_off_vel += RvR_fix16_mul(player.entity->vz - player.vis_off_vel, 28762);
   RvR_fix16 dz = player.entity->z + CAMERA_COLL_HEIGHT_BELOW - player.cam.z;
   player.vis_off_vel += RvR_fix16_mul(dz * 64, 6144);
   player.cam.z += player.vis_off_vel / 64;
   //-------------------------------------

   //Update cam position and direction
   player.cam.x = player.entity->x;
   player.cam.y = player.entity->y;
   player.cam.dir = player.entity->direction;
   player.cam.fov = 16384;
   player.cam.shear = player.shear;

   //View bobbing
   RvR_fix16 vel_len = RvR_fix16_sqrt(RvR_fix16_mul(player.entity->vx, player.entity->vx) + RvR_fix16_mul(player.entity->vy, player.entity->vy));
   RvR_fix16 bob_factor = vel_len / 1024;
   player.cam.z += RvR_fix16_mul(RvR_fix16_sin(game_tick * 2184), bob_factor);
   //-------------------------------------
}
//-------------------------------------
