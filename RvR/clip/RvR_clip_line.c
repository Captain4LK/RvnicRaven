/*
RvnicRaven - 2d line clipping

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvR/RvR_config.h"
#include "RvR/RvR_math.h"
#include "RvR/RvR_fix24.h"
#include "RvR/RvR_clip.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
uint8_t outcode(RvR_fix24 l, RvR_fix24 u, RvR_fix24 r, RvR_fix24 d, RvR_fix24 x, RvR_fix24 y);
//-------------------------------------

//Function implementations

//Cohen–Sutherland line clipping
int RvR_clip_line(RvR_fix24 l, RvR_fix24 u, RvR_fix24 r, RvR_fix24 d, RvR_fix24 *x0, RvR_fix24 *y0, RvR_fix24 *x1, RvR_fix24 *y1)
{
   uint8_t code0 = outcode(l,u,r,d,*x0,*y0);
   uint8_t code1 = outcode(l,u,r,d,*x1,*y1);

   for(;;)
   {
      if(code0==0&&code1==0)
         return 1;

      if(code0&code1)
         return 0;

      uint8_t code_out = code1>code0?code1:code0;
      RvR_fix24 x,y;
      RvR_fix24 dx = *x1-*x0;
      RvR_fix24 dy = *y1-*y0;

      if(code_out&8)
      {
         x = *x0+RvR_fix24_div(RvR_fix24_mul(dx,d-*y0),dy);
         y = d;
      }
      else if(code_out&4)
      {
         x = *x0+RvR_fix24_div(RvR_fix24_mul(dx,u-*y0),dy);
         y = u;
      }
      else if(code_out&2)
      {
         x = r;
         y = *y0+RvR_fix24_div(RvR_fix24_mul(dy,r-*x0),dx);
      }
      else
      {
         x = l;
         y = *y0+RvR_fix24_div(RvR_fix24_mul(dy,l-*x0),dx);
      }

      if(code_out==code0)
      {
         *x0 = x;
         *y0 = y;
         code0 = outcode(l,u,r,d,*x0,*y0);
      }
      else
      {
         *x1 = x;
         *y1 = y;
         code1 = outcode(l,u,r,d,*x1,*y1);
      }
   }

   return 0;
}

uint8_t outcode(RvR_fix24 l, RvR_fix24 u, RvR_fix24 r, RvR_fix24 d, RvR_fix24 x, RvR_fix24 y)
{
   uint8_t code = 0;

   if(x<l)
      code|=1;
   else if(x>r)
      code|=2;

   if(y<u)
      code|=4;
   if(y>d)
      code|=8;

   return code;
}
//-------------------------------------
