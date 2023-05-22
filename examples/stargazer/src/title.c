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
//-------------------------------------

//Internal includes
#include "config.h"
#include "state.h"
#include "title.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int select = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void title_update()
{
   if(RvR_key_pressed(config_strafe_left)||RvR_key_pressed(RVR_KEY_LEFT))
      select = RvR_max(0,select-1);
   if(RvR_key_pressed(config_strafe_right)||RvR_key_pressed(RVR_KEY_RIGHT))
      select = RvR_min(4,select+1);

   //TODO: config_shoot key?
   if(RvR_key_pressed(RVR_KEY_ENTER))
   {
      switch(select)
      {
      case 0: state_init(STATE_GAME); return;
      case 1: return;
      case 2: return;
      case 3: return;
      case 4: RvR_quit(); return;
      }
   }
}

void title_draw()
{
   //Terrible assumption of texture dimensions
   RvR_texture *tex = RvR_texture_get(57344);
   memcpy(RvR_framebuffer(),tex->data,RvR_xres()*RvR_yres());

   //Title
   //TODO: replace
   RvR_render_string((RvR_xres()-250)/2,8,5,"RvnicRaven",24);
   RvR_render_string((RvR_xres()-180)/2,64,4,"stargazer",24);

   //Selection
   if(select==0) RvR_render_string((RvR_xres()/5-100)/2,RvR_yres()-64,2,">New game<",24);
   else          RvR_render_string((RvR_xres()/5-80)/2,RvR_yres()-64,2,"New game",20);

   if(select==1) RvR_render_string((RvR_xres()/5-100)/2+RvR_xres()/5,RvR_yres()-64,2,">Continue<",24);
   else          RvR_render_string((RvR_xres()/5-80)/2+RvR_xres()/5,RvR_yres()-64,2,"Continue",20);

   if(select==2) RvR_render_string((RvR_xres()/5-95)/2+2*(RvR_xres()/5),RvR_yres()-64,2,">Options<",24);
   else          RvR_render_string((RvR_xres()/5-75)/2+2*(RvR_xres()/5),RvR_yres()-64,2,"Options",20);

   if(select==3) RvR_render_string((RvR_xres()/5-95)/2+3*(RvR_xres()/5),RvR_yres()-64,2,">Credits<",24);
   else          RvR_render_string((RvR_xres()/5-75)/2+3*(RvR_xres()/5),RvR_yres()-64,2,"Credits",20);

   if(select==4) RvR_render_string((RvR_xres()/5-60)/2+4*(RvR_xres()/5),RvR_yres()-64,2,">Quit<",24);
   else          RvR_render_string((RvR_xres()/5-40)/2+4*(RvR_xres()/5),RvR_yres()-64,2,"Quit",20);
}

void title_init()
{
}

void title_set()
{
   RvR_mouse_relative(0);
}
//-------------------------------------
