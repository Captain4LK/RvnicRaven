/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _SPRITE_H_

#define _SPRITE_H_

typedef struct
{
   void *owner;
}Sprite;

void sprites_init();
int sprite_valid(uint16_t id, void *owner);
uint16_t sprite_new(void *owner);
uint16_t sprite_texture(uint16_t id);

//Drawing
void sprite_clear(uint16_t id, uint8_t color);
void sprite_draw_sprite(uint16_t id, uint16_t tex, int x, int y, int sx, int sy, int width, int height);
void sprite_draw_sprite_remap(uint16_t id, uint16_t tex, int x, int y, int sx, int sy, int width, int height, uint8_t r0, uint8_t r1, uint8_t r2, uint8_t r3);

#endif
