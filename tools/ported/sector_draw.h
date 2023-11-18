/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _SECTOR_DRAW_H_

#define _SECTOR_DRAW_H_

void sector_draw_start(RvR_fix22 x, RvR_fix22 y);
int sector_draw_add(RvR_fix22 x, RvR_fix22 y);
void sector_draw_draw(RvR_fix22 x, RvR_fix22 y, int grid_size);

#endif
