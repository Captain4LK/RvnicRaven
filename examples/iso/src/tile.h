/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _TILE_H_

#define _TILE_H_

int tile_has_wall(uint32_t tile);
int tile_has_floor(uint32_t tile);
int tile_has_object(uint32_t tile);
int tile_has_draw_wall(uint32_t tile);
int tile_has_draw_floor(uint32_t tile);
int tile_has_draw_slope(uint32_t tile);
int tile_is_slope(uint32_t tile);
int tile_visible(uint32_t tile);
int tile_discovered(uint32_t tile);

uint32_t tile_set_visible(uint32_t tile, int visible);
uint32_t tile_set_discovered(uint32_t tile, int discovered);

uint32_t tile_make_wall(uint16_t wall, uint16_t floor);
uint32_t tile_make_object(uint16_t object, uint16_t floor);
uint32_t tile_make_slope(uint16_t slope, uint16_t variant);

uint16_t tile_wall_texture(uint32_t tile);
uint16_t tile_object_texture(uint32_t tile);
uint16_t tile_floor_texture(uint32_t tile);
uint16_t tile_slope_texture(uint32_t tile, uint8_t rotation);

#endif
