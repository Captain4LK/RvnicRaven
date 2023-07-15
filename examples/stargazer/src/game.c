/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
#include "game.h"
#include "map.h"
#include "ai.h"
#include "state.h"
#include "message.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
uint32_t game_tick = 0;

static int fade_timer;
static uint32_t fade_state;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_update()
{
   game_tick++;

   player_update();

   Entity *e = entities;
   while(e!=NULL)
   {
      Entity *next = e->next;

      if(e==player.entity)
         goto next;

      ai_run(e);

next:
      e = next;
   }

   //Free entities tagged 'removed'
   e = entities;
   while(e!=NULL)
   {
      Entity *next = e->next;

      if(e->removed)
         entity_free(e);

      e = next;
   }

   //Free cards tagged 'removed'
   Card *c = cards;
   while(c!=NULL)
   {
      Card *next = c->next;

      if(c->removed)
         card_free(c);

      c = next;
   }

   if(RvR_key_pressed(RVR_KEY_T))
      state_set(STATE_GAME_CARD);
}

void game_draw()
{
   if(fade_timer==FADE_TIME)
      memcpy(RvR_texture_get(65535)->data, RvR_framebuffer(), RvR_xres() * RvR_yres());

   RvR_ray_draw_begin();
   sprite_draw_begin();

   //Draw entities
   Entity *e = entities;
   while(e!=NULL)
   {
      sprite_draw(e->x, e->y, e->z, e->direction, e->sprite, NULL);
      e = e->next;
   }

   //Draw cards
   Card *c = cards;
   while(c!=NULL)
   {
      sprite_draw(c->x, c->y, c->z, 0, 32768, NULL);
      c = c->next;
   }

   RvR_ray_draw_map(&player.cam, map_current());

   sprite_draw_end();

   RvR_ray_draw_end(&player.cam, map_current(), NULL);

   if(fade_timer--)
   {
      RvR_texture *fade = RvR_texture_get(65535);
      RvR_render_texture(fade, 0, 0);
      for(int i = 0; i<(1 << 19) / FADE_TIME; i++)
      {
         fade_state = (17 * fade_state + 33) & ((1 << 19) - 1);
         if(fade_state<RvR_xres() * RvR_yres())
            fade->data[fade_state] = 0;
      }
   }

   char tmp[128];
   snprintf(tmp, 128, "%d", entity_health(player.entity));
   RvR_render_string(4, RvR_yres() - 32, 3, tmp, 30);

   message_draw(31);
}

void game_init()
{
   map_load(0);
   map_init();
   message_reset();
}

void game_set()
{
   RvR_mouse_relative(1);
   fade_timer = FADE_TIME;
   fade_state = 0; //TODO: random number
}
//-------------------------------------
