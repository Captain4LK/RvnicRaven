/*
RvnicRaven - general math utilities

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_MATH_H_

#define _RVR_MATH_H_

#include <stdint.h>

#define RvR_min(a, b) ((a)<(b)?(a):(b))
#define RvR_max(a, b) ((a)>(b)?(a):(b))
#define RvR_abs(a) ((a)<0?-(a):(a))
#define RvR_non_zero(a) ((a) + ((a)==0))
#define RvR_clamp(a, min, max) (RvR_max((min), RvR_min((max), (a))))

#define RvR_sign(a) ((a)>=0?1:-1)
#define RvR_sign_equal(a, b) (RvR_sign(a)==RvR_sign(b))

inline uint32_t RvR_log2(uint32_t a)
{
   //Use builtin if availible
   //x86 has bsr instruction for this
#if defined __has_builtin && __has_builtin (__builtin_clz)
   if(a==0) return 0;
   return 31-__builtin_clz(a);
#else
   uint32_t n = 1;
   if(a==0) return 0; //this is technically incorrect, but doing this would invoke ub in a bunch of places (often: 1<<RvR_log(2))
   if((a>>16)==0) { n+=16; a<<=16; }
   if((a>>24)==0) { n+=8;  a<<=8; }
   if((a>>28)==0) { n+=4;  a<<=4; }
   if((a>>30)==0) { n+=2;  a<<=2; }
   n = n-(a>>31);

   return 31-n;
#endif
}

#endif
