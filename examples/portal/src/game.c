/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "state.h"
#include "game.h"
#include "gamestate.h"
#include "player.h"
#include "bookcase.h"
#include "book.h"
#include "color.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Gamestate *gamestate = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_set()
{
   gamestate_free(gamestate);
   gamestate = gamestate_new();
   player_create_new(gamestate);
   gamestate_map(gamestate, 0);
}

void game_init()
{
}

void game_update()
{
   player_update(gamestate);
}

void game_draw()
{
   RvR_port_draw_begin(gamestate->map, &gamestate->cam);

   gamestate->select.x = RvR_xres()/2;
   gamestate->select.y = RvR_yres()/2;
   RvR_port_draw_map(&gamestate->select);

   for(int i = 0;i<gamestate->map->sprite_count;i++)
   {
      RvR_port_draw_sprite(i,NULL);
   }

   RvR_port_draw_end(NULL);

   RvR_render_rectangle_fill(RvR_xres()/2-1,RvR_yres()/2-1,2,2,1);

   //
   uint16_t book_hover = BOOK_INVALID;
   if((gamestate->select.type==RVR_PORT_SWALL_BOT))
   {
      RvR_port_wall *wall = gamestate->map->walls+gamestate->select.as.wall;
      if(wall->tex_lower>(UINT16_MAX-BOOKCASE_COUNT)&&gamestate->select.depth<2048)
      {
         int tx = gamestate->select.tx;
         int ty = gamestate->select.ty;

         if(tx<4||tx>=60)
            book_hover = BOOK_INVALID;
         else if(ty>=8&&ty<=24)
            book_hover = bookcase_at(-(wall->tex_lower-UINT16_MAX),0,(tx-4)/4);
         else if(ty>=28&&ty<=48)
            book_hover = bookcase_at(-(wall->tex_lower-UINT16_MAX),1,(tx-4)/4);
         else if(ty>=52&&ty<=72)
            book_hover = bookcase_at(-(wall->tex_lower-UINT16_MAX),2,(tx-4)/4);
      }
   }

   if(book_hover!=BOOK_INVALID)
   {
      Book *book = book_get(book_hover);
      int len = strlen(book->title);
      RvR_render_rectangle_fill(RvR_xres()/2-(len*5)/2-2,RvR_yres()-18,len*5+4,12,color_light_gray);
      RvR_render_string(RvR_xres()/2-(len*5)/2,RvR_yres()-16,1,book->title,color_black);
   }
}
//-------------------------------------
