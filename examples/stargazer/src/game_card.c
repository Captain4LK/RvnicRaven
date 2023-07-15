/*
RvnicRaven - stargazer

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "card_text.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
#include "game_card.h"
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
RvR_ray_selection select;

Card *selected;

static int fade_timer;
static uint32_t fade_state;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_card_update()
{
   if(RvR_key_pressed(RVR_KEY_T))
      state_set(STATE_GAME);

   if(RvR_key_pressed(RVR_KEY_TAB))
      state_set(STATE_GAME_INVENTORY);

   if(select.ref==NULL)
      return;

   Card *card = (Card *)select.ref;
   if(RvR_key_pressed(RVR_BUTTON_LEFT))
   {
      selected = card;
      state_set(STATE_GAME_CARD_VIEW);
   }
}

void game_card_draw()
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
   for(; c!=NULL; c = c->next)
   {
      if(c->removed)
         continue;
      sprite_draw(c->x, c->y, c->z, 0, 32768, c);
   }

   RvR_ray_draw_map(&player.cam, map_current());

   sprite_draw_end();

   int mx, my;
   RvR_mouse_pos(&mx, &my);
   select.x = mx;
   select.y = my;
   RvR_ray_draw_end(&player.cam, map_current(), &select);

   RvR_render_rectangle_fill(0, 0, RvR_xres(), 8, 20);
   RvR_render_rectangle_fill(0, 0, 8, RvR_yres(), 20);
   RvR_render_rectangle_fill(0, RvR_yres() - 8, RvR_xres(), 8, 20);
   RvR_render_rectangle_fill(RvR_xres() - 8, 0, 8, RvR_yres(), 20);

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
}

void game_card_init()
{}

void game_card_set()
{
   RvR_mouse_relative(0);
   fade_timer = FADE_TIME;
   fade_state = 0; //TODO: random number
}

void game_card_view_update()
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_pressed(RVR_BUTTON_LEFT))
   {
      if(mx<RvR_xres() / 2 - 96||mx>RvR_xres() / 2 + 96||my<RvR_yres() / 2 - 128||my>RvR_yres() / 2 + 128)
      {
         state_set(STATE_GAME_CARD);
         return;
      }

      //Check for empty inventory space
      for(int i = 0; i<player.entity->cards_size; i++)
      {
         if(player.entity->cards[i].type==CARD_NONE)
         {
            card_copy(&player.entity->cards[i], selected);
            card_remove(selected);
            selected = NULL;
            state_set(STATE_GAME_CARD);
            return;
         }
      }

      //No space left --> STATE_GAME_CARD_INVENTORY
   }
}

void game_card_view_draw()
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
   for(; c!=NULL; c = c->next)
   {
      if(c->removed)
         continue;
      sprite_draw(c->x, c->y, c->z, 0, 32768, NULL);
   }

   RvR_ray_draw_map(&player.cam, map_current());

   sprite_draw_end();

   RvR_ray_draw_end(&player.cam, map_current(), NULL);

   RvR_render_rectangle_fill(RvR_xres() / 2 - 96, RvR_yres() / 2 - 128, 192, 256, 10);

   const char *title = card_title(selected);
   RvR_render_string(RvR_xres() / 2 - strlen(title) * 3, RvR_yres() / 2 - 128, 1, title, 24);

   uint16_t tex = 33280 + 13 * selected->type + (selected->rank - 1);
   RvR_texture *texture = RvR_texture_get(tex);
   RvR_render_texture(texture, RvR_xres() / 2 - texture->width / 2, RvR_yres() / 2 - texture->height / 2 - texture->height);

   RvR_render_rectangle_fill(0, 0, RvR_xres(), 8, 20);
   RvR_render_rectangle_fill(0, 0, 8, RvR_yres(), 20);
   RvR_render_rectangle_fill(0, RvR_yres() - 8, RvR_xres(), 8, 20);
   RvR_render_rectangle_fill(RvR_xres() - 8, 0, 8, RvR_yres(), 20);

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
}

void game_card_view_init()
{}

void game_card_view_set()
{
   fade_timer = FADE_TIME;
   fade_state = 0; //TODO: random number
}

void game_card_inventory_update()
{}

void game_card_inventory_draw()
{}

void game_card_inventory_init()
{}

void game_card_inventory_set()
{}
//-------------------------------------
