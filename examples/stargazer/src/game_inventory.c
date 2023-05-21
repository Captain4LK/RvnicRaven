/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../RvR/RvnicRaven.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "card.h"
#include "sprite.h"
#include "entity.h"
#include "player.h"
#include "game.h"
#include "game_inventory.h"
#include "state.h"
#include "map.h"
#include "ai.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static RvR_fix22 dist_table[RVR_XRES*RVR_YRES*2*2];
static RvR_fix22 ang_table[RVR_XRES*RVR_YRES*2*2];
static int fade_timer;
static uint32_t fade_state;

static int card_drag = -1;
static int card_drag_x;
static int card_drag_y;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_inventory_update()
{

   if(RvR_core_key_pressed(config_inventory))
      state_set(STATE_GAME);

   if(RvR_core_key_pressed(RVR_BUTTON_LEFT)&&card_drag==-1)
   {
      int mx,my;
      RvR_core_mouse_pos(&mx,&my);
      mx-=10;
      my-=10;

      int slot_x = (mx)/52;
      int slot_y = (my)/73;
      int slot = slot_y*12+slot_x;
      if(mx>0&&mx<624&&my>0&&my<438&&mx-slot_x*52<48&&my-slot_y*73<69&&slot<player.entity->cards_size&&player.entity->cards[slot].type!=CARD_NONE)
      {
         card_drag = slot;
         card_drag_x = slot_x*52-mx;
         card_drag_y = slot_y*73-my;
      }
   }
   if(RvR_core_key_released(RVR_BUTTON_LEFT)&&card_drag!=-1)
   {
      int mx,my;
      RvR_core_mouse_pos(&mx,&my);
      mx-=10;
      my-=10;

      int slot_x = mx/52;
      int slot_y = my/73;
      int slot = slot_y*12+slot_x;
      if(mx>0&&mx<624&&my>0&&my<438&&mx-slot_x*52<48&&my-slot_y*73<69&&slot<player.entity->cards_size)
      {
         if(slot==card_drag)
         {
            //TODO: click on card, show card info
         }
         else
         {
            //Swap cards
            Card tmp;
            memcpy(&tmp,&player.entity->cards[card_drag],sizeof(tmp));
            memcpy(&player.entity->cards[card_drag],&player.entity->cards[slot],sizeof(tmp));
            memcpy(&player.entity->cards[slot],&tmp,sizeof(tmp));
         }
      }

      card_drag = -1;
   }
}

void game_inventory_draw()
{
   int mx,my;
   RvR_core_mouse_pos(&mx,&my);

   //Tunnel effect
   if(fade_timer==FADE_TIME)
      memcpy(RvR_texture_get(65535)->data,RvR_core_framebuffer(),RVR_XRES*RVR_YRES);

   static int tick = 0;
   static RvR_fix22 look_x = RVR_XRES*512;
   static RvR_fix22 look_y = RVR_YRES*512;

   tick++;
   RvR_fix22 shift_x = tick*256;
   RvR_fix22 shift_y = tick*512;
   look_x = (look_x+(mx*1024-look_x)/64);
   look_y = (look_y+(my*1024-look_y)/64);

   const uint8_t * restrict src = RvR_texture_get(2054)->data;
   uint8_t * restrict dst = RvR_core_framebuffer();
   const RvR_fix22 * restrict dist = &dist_table[(look_y/1024)*RVR_XRES*2+look_x/1024];
   const RvR_fix22 * restrict ang = &ang_table[(look_y/1024)*RVR_XRES*2+look_x/1024];
   for(int y = 0;y<RVR_YRES;y++)
   {
      for(int x = 0;x<RVR_XRES;x++)
      {
         int shade = (*dist)/2048;
         int distance = (((*dist)+shift_x)/1024)&63;
         int angle = (((*ang)+shift_y)/1024)&63;
         *dst = RvR_shade_table(shade)[src[distance+angle*64]];

         dist++;
         ang++;
         dst++;
      }

      dist+=RVR_XRES;
      ang+=RVR_XRES;
   }
   //-------------------------------------

   for(int i = 0;i<player.entity->cards_size;i++)
   {
      int x = (i%12)*52;
      int y = (i/12)*73;

      uint16_t tex = 33024;
      if(player.entity->cards[i].type!=CARD_NONE&&i!=card_drag)
         tex = 33280+13*player.entity->cards[i].type+player.entity->cards[i].rank;

      RvR_draw_texture(RvR_texture_get(tex),10+x,10+y);
   }
   if(card_drag!=-1)
   {
      uint16_t tex = 33280+13*player.entity->cards[card_drag].type+player.entity->cards[card_drag].rank;
      RvR_draw_texture(RvR_texture_get(tex),mx+card_drag_x,my+card_drag_y);
   }
   
   if(fade_timer--)
   {
      RvR_texture *fade = RvR_texture_get(65535);
      RvR_draw_texture(fade,0,0);
      for(int i = 0;i<(1<<19)/FADE_TIME;i++)
      {
         fade_state = (17*fade_state+33)&((1<<19)-1);
         if(fade_state<RVR_XRES*RVR_YRES)
            fade->data[fade_state] = 0;
      }
   }
}

void game_inventory_init()
{
   for(int y = 0;y<RVR_YRES*2;y++)
   {
      for(int x = 0;x<RVR_XRES*2;x++)
      {
         double root = sqrt((x-RVR_XRES)*(x-RVR_XRES)+(y-RVR_YRES)*(y-RVR_YRES));
         double dist = 128.0;
         if(root!=0.0)
            dist = (32.0*64.0)/sqrt((x-RVR_XRES)*(x-RVR_XRES)+(y-RVR_YRES)*(y-RVR_YRES));
         dist_table[y*RVR_XRES*2+x] = RvR_min((RvR_fix22)(dist*1024),131071);
         double ang = (0.5*64*atan2(y-RVR_YRES,x-RVR_XRES))/3.14159265358979323846;
         ang_table[y*RVR_XRES*2+x] = ang*1024;
      }
   }
}

void game_inventory_set()
{
   RvR_core_mouse_relative(0);
   fade_timer = FADE_TIME;
   fade_state = 0; //TODO: random number
   card_drag = -1;
}
//-------------------------------------
