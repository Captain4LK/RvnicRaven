/*
RvnicRaven - 2d clipping

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_CLIP_H_

#define _RVR_CLIP_H_

int RvR_clip_line(RvR_fix24 l, RvR_fix24 u, RvR_fix24 r, RvR_fix24 d, RvR_fix24 *x0, RvR_fix24 *y0, RvR_fix24 *x1, RvR_fix24 *y1);

#endif
