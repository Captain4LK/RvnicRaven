/*
RvnicRaven - general math utilities

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#define RvR_sign_equal(a,b) (((a)^(b))>=0)

uint32_t RvR_log2(uint32_t a);

#endif
