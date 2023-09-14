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
#include "config.h"
#include "draw.h"
#include "log.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static char log_buffer[LOG_LENGTH][64];
int log_buffer_next = 0;
int log_buffer_len = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void log_draw(int scroll)
{
   draw_fill_rectangle(0, RvR_yres() - 74, RvR_xres(),74, 1, 1);
   draw_line_horizontal(0,RvR_xres(),RvR_yres()-75,42,1);
   for(int i = 0;i<RvR_min(log_buffer_len,9);i++)
   {
      RvR_render_string(RvR_xres()-256+1,RvR_yres()-74+1+(8-i)*8,1,log_buffer[((log_buffer_next-i-1)%LOG_LENGTH+LOG_LENGTH)%LOG_LENGTH],42);
   }
}

void log_push(const char *text)
{
   int len_cur = 0;
   for(int i = 0; text[i]&&i<1024; i++)
   {
      if(text[i]==' ')
      {
         //Search next space (or newline)
         int len = 1;
         for(; text[i + len]!=' '&&text[i + len]!='\n'&&text[i + len]!='\0'; len++);
         if(len_cur + len - 1>=52)
         {
            log_buffer[log_buffer_next][len_cur] = '\0';
            log_buffer_next = ((log_buffer_next+1)%LOG_LENGTH+LOG_LENGTH)%LOG_LENGTH;
            log_buffer_len = RvR_min(LOG_LENGTH,log_buffer_len+1);
            len_cur = 0;
            continue;
         }
      }

      if(text[i]=='\n'||len_cur>=52)
      {
         log_buffer[log_buffer_next][len_cur] = '\0';
         log_buffer_next = ((log_buffer_next+1)%LOG_LENGTH+LOG_LENGTH)%LOG_LENGTH;
         log_buffer_len = RvR_min(LOG_LENGTH,log_buffer_len+1);
         len_cur = 0;
         continue;
      }

      log_buffer[log_buffer_next][len_cur] = text[i];
      len_cur++;
   }

   if(len_cur!=0)
   {
      log_buffer[log_buffer_next][len_cur] = '\0';
      log_buffer_next = ((log_buffer_next+1)%LOG_LENGTH+LOG_LENGTH)%LOG_LENGTH;
      log_buffer_len = RvR_min(LOG_LENGTH,log_buffer_len+1);
   }
}

void log_clear()
{
   log_buffer_next = log_buffer_len = 0;
}
//-------------------------------------
