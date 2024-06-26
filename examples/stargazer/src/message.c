/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvR/RvR.h"
#include "config.h"
#include "game.h"
#include "message.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
struct
{
   char msg[64];
   uint32_t tick_timeout;
   int next;
} messages[MESSAGE_MAX] = {[0].next = -1};

int message_first = -1;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void message_reset()
{
   memset(messages, 0, sizeof(messages));
   messages[0].next = -1;
   message_first = -1;
}

void message_queue(const char *msg)
{
   //Add as first message if possible
   if(message_first==-1)
   {
      message_first = 0;
      messages[0].next = -1;
      strncpy(messages[0].msg, msg, 64);
      messages[0].tick_timeout = game_tick + MESSAGE_TIMEOUT;

      return;
   }

   //Find last message
   int msg_last = message_first;
   for(int i = 0; i<MESSAGE_MAX&&messages[msg_last].next!=-1; i++)
      msg_last = messages[msg_last].next;

   //Search for free mesage
   int msg_free = -1;
   for(int i = 0; i<MESSAGE_MAX; i++)
   {
      if(messages[i].tick_timeout<game_tick)
      {
         msg_free = i;
         break;
      }
   }

   //Add message in free slot
   if(msg_free!=-1)
   {
      messages[msg_free].tick_timeout = game_tick + MESSAGE_TIMEOUT;
      messages[msg_free].next = -1;
      strncpy(messages[msg_free].msg, msg, 64);
      messages[msg_last].next = msg_free;

      return;
   }

   //Remove first mesage and queue up
   int old_first = message_first;
   message_first = messages[message_first].next;
   messages[old_first].tick_timeout = game_tick + MESSAGE_TIMEOUT;
   messages[old_first].next = -1;
   strncpy(messages[old_first].msg, msg, 64);
   messages[msg_last].next = old_first;
}

void message_draw(uint8_t index)
{
   if(message_first==-1)
      return;

   //Remove timed out messages
   for(int i = 0; i<MESSAGE_MAX&&message_first!=-1&&messages[message_first].tick_timeout<game_tick; i++)
      message_first = messages[message_first].next;

   //Draw messages
   int message = message_first;
   int y = 2;
   for(int i = 0; i<MESSAGE_MAX&&message!=-1; i++)
   {
      RvR_render_string(2, y, 1, messages[message].msg, index);
      message = messages[message].next;
      y += 8;
   }
}
//-------------------------------------
