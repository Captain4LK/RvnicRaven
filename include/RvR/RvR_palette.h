/*
RvnicRaven - palette and color luts

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PALETTE_H_

#define _RVR_PALETTE_H_

typedef struct
{
   uint8_t r, g, b, a;
}RvR_color;

void       RvR_palette_load(uint16_t id);
RvR_color *RvR_palette();
uint8_t   *RvR_shade_table(uint8_t light);
uint8_t    RvR_blend(uint8_t c0, uint8_t c1);

#endif
