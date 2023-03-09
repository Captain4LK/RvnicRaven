/*
RvnicRaven - rendering: lines

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR/RvR_app.h"
#include "RvR/RvR_math.h"
#include "RvR/RvR_texture.h"
#include "RvR/RvR_fix24.h"
#include "RvR/RvR_clip.h"
#include "RvR/RvR_render.h"
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

void RvR_render_line(RvR_fix24 x0, RvR_fix24 y0, RvR_fix24 x1, RvR_fix24 y1, uint8_t index)
{
   if(!RvR_clip_line(0,0,RVR_XRES*256-1,RVR_YRES*256-1,&x0,&y0,&x1,&y1))
      return;

   RvR_fix24 dx = x1-x0;
   RvR_fix24 dy = y1-y0;
   int x = x0>>8;
   int y = y0>>8;
   RvR_fix24 db;
   RvR_fix24 ds;
   RvR_fix24 fracb;
   RvR_fix24 fracs;
   int steps;
   int stepb;
   int togox = (x1>>8)-(x0>>8);
   int togoy = (y1>>8)-(y0>>8);
   int togob;
   int togos;

   if(dx>=0)
   {
      if(dy>=0)
      {
         db = dx;
         ds = dy;
         fracb = x0&255;
         fracs = y0&255;
         togob = togox;
         togos = togoy;
         stepb = 1;
         steps = RVR_XRES;
      }
      else
      {
         db = dx;
         ds = -dy;
         fracb = x0&255;
         fracs = 255-(y0&255);
         togob = togox;
         togos = -togoy;
         stepb = 1;
         steps = -RVR_XRES;
      }
   }
   else
   {
      if(dy>=0)
      {
         db = -dx;
         ds = dy;
         fracb = 255-(x0&255);
         fracs = y0&255;
         togob = -togox;
         togos = togoy;
         stepb = -1;
         steps = RVR_XRES;
      }
      else
      {
         db = -dx;
         ds = -dy;
         fracb = 255-(x0&255);
         fracs = 255-(y0&255);
         togob = -togox;
         togos = -togoy;
         stepb = -1;
         steps = -RVR_XRES;
      }
   }

   if(db<ds)
   {
      int32_t tmp;
      tmp = db; db = ds; ds = tmp;
      tmp = fracb; fracb = fracs; fracs = tmp;
      tmp = togob; togob = togos; togos = tmp;
      tmp = stepb; stepb = steps; steps = tmp;
   }

   uint8_t * restrict dst = &RvR_framebuffer()[y * RVR_XRES + x];
   RvR_fix24 dist = RvR_fix24_mul(fracs-128,db)-RvR_fix24_mul(fracb-128,ds);
   int togo = togob;
   while(togo>0)
   {
      if(dist>db/2)
      {
         dst+=steps;
         dist-=db;
      }

      *dst = index;
      dst+=stepb;
      dist+=ds;
      togo--;
   }
}
//-------------------------------------
