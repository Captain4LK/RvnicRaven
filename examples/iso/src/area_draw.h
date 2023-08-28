/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _AREA_DRAW_H_

#define _AREA_DRAW_H_

#include "world_defs.h"
#include "camera.h"

//Passed data must be valid until area_draw_end()
void area_draw_begin(const World *w, const Area *a, const Camera *c);
void area_draw_end();

void area_draw_sprite(uint16_t tex, int16_t x, int16_t y, int16_t z);

#endif
