/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _DRAW_H_

#define _DRAW_H_

void draw_fill_rectangle(int x, int y, int width, int height, uint8_t col, int transparent);
void draw_string_wrap(int x, int y, int width, int height, int scale, const char *string, uint8_t col);

#endif
