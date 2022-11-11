#ifndef _RVR_MATH_H_

/*
   RvnicRaven - basic math routines

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
   To create implementation (the function definitions) add
      #define RVR_DRAW_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_MATH_H_

//General purpose function macros
//NOT TAKING INTO ACCOUNT SIDE EFFECTS, USE WITH CARE
#define RvR_max(a, b) ((a)>(b)?(a):(b))
#define RvR_min(a, b) ((a)<(b)?(a):(b))
#define RvR_abs(a) ((a)<0?-(a):(a))
#define RvR_non_zero(a) ((a) + ((a)==0))
#define RvR_sign_equal(a, b) (((a) ^ (b))>=0)
#define RvR_clamp(a, min, max) (RvR_max((min), RvR_min((max), (a))))

int32_t  RvR_div_round_down(int32_t a, int32_t b);
uint32_t RvR_log2(uint32_t a);

#endif

#ifdef RVR_MATH_IMPLEMENTATION
#ifndef RVR_MATH_IMPLEMENTATION_ONCE
#define RVR_MATH_IMPLEMENTATION_ONCE

int32_t RvR_div_round_down(int32_t a, int32_t b)
{
   return a / b - ((a>=0)?0:1);
}

uint32_t RvR_log2(uint32_t a)
{
   uint32_t r = 0;
   while(a >>= 1) r++;

   return r;
}

#endif
#endif
