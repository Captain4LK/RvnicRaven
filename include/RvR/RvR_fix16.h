/*
RvnicRaven - 16.16 fixed point math

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_FIX16_H_

#define _RVR_FIX16_H_

typedef int32_t RvR_fix16;

RvR_fix16 RvR_fix16_mul(RvR_fix16 a, RvR_fix16 b);
RvR_fix16 RvR_fix16_div(RvR_fix16 a, RvR_fix16 b);

RvR_fix16 RvR_fix16_cos(RvR_fix16 a);
RvR_fix16 RvR_fix16_sin(RvR_fix16 a);
RvR_fix16 RvR_fix16_tan(RvR_fix16 a);
RvR_fix16 RvR_fix16_atan2(RvR_fix16 y, RvR_fix16 x);

RvR_fix16 RvR_fix16_sqrt(RvR_fix16 a);

#endif
