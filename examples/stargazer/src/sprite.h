/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _SPRITE_H_

#define _SPRITE_H_

typedef struct
{
   uint8_t rot;
   uint16_t tex[8];
   uint16_t flag[8];
}Sprite;

extern Sprite sprites[1<<16];

void sprites_init(void);
void sprite_draw_begin();
void sprite_draw(RvR_vec3 pos, RvR_fix22 dir, int32_t sprite);
void sprite_draw_end();

#endif
