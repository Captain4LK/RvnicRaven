/*
RvnicRaven - 22.10 fixed point math

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_FIX22_H_

#define _RVR_FIX22_H_

typedef int32_t RvR_fix22;

RvR_fix22 RvR_fix22_mul(RvR_fix22 a, RvR_fix22 b);
RvR_fix22 RvR_fix22_div(RvR_fix22 a, RvR_fix22 b);

RvR_fix22 RvR_fix22_cos(RvR_fix22 a);
RvR_fix22 RvR_fix22_sin(RvR_fix22 a);
RvR_fix22 RvR_fix22_tan(RvR_fix22 a);
RvR_fix22 RvR_fix22_atan2(RvR_fix22 y, RvR_fix22 x);
RvR_fix22 RvR_fix22_atan2_slow(RvR_fix22 y, RvR_fix22 x);

RvR_fix22 RvR_fix22_sqrt(RvR_fix22 a);

#endif
