/*
RvnicRaven - rendering

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_RENDER_H_

#define _RVR_RENDER_H_

void RvR_render_clear(uint8_t index);
void RvR_render_texture(RvR_texture *t, int x, int y);
void RvR_render_texture2(RvR_texture *t, int x, int y);
void RvR_render_rectangle(int x, int y, int width, int height, uint8_t index);
void RvR_render_rectangle_fill(int x, int y, int width, int height, uint8_t index);
void RvR_render_circle(int x, int y, int radius, uint8_t index);
void RvR_render_font_set(uint16_t id);
void RvR_render_string(int x, int y, int scale, const char *text, uint8_t index);
void RvR_render(int x, int y, uint8_t index); //do not use, access framebuffer directly if possible
void RvR_render_line(RvR_fix24 x0, RvR_fix24 y0, RvR_fix24 x1, RvR_fix24 y1, uint8_t index);
void RvR_render_vertical_line(int x, int y0, int y1, uint8_t index);
void RvR_render_horizontal_line(int x0, int x1, int y, uint8_t index);

#endif
