/*
RvnicRaven - raycast

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_RAY_H_

#define _RVR_RAY_H_

typedef struct RvR_ray_depth_buffer_entry RvR_ray_depth_buffer_entry;

typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 dir;
   RvR_fix16 shear;
   RvR_fix16 fov;
}RvR_ray_cam;

typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 dirx;
   RvR_fix16 diry;
}RvR_ray;

typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 direction;
   uint16_t texture;
   uint32_t flags;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
}RvR_ray_map_sprite;

typedef struct
{
   uint16_t sky_tex;
   uint16_t width;
   uint16_t height;
   uint32_t sprite_count;
   RvR_fix16 *floor;
   RvR_fix16 *ceiling;
   uint16_t *floor_tex;
   uint16_t *ceil_tex;
   uint16_t *wall_ftex;
   uint16_t *wall_ctex;
   RvR_ray_map_sprite *sprites;
}RvR_ray_map;

typedef struct
{
   RvR_fix16 posx;
   RvR_fix16 posy;
   RvR_fix16 posz;
   RvR_fix16 squarex;
   RvR_fix16 squarey;
   RvR_fix16 distance;
   uint8_t direction;
   uint16_t wall_ftex;
   uint16_t wall_ctex;
   uint16_t floor_tex;
   uint16_t ceil_tex;
   RvR_fix16 fheight;
   RvR_fix16 cheight;
   RvR_fix16 texture_coord;
}RvR_ray_hit_result;

typedef struct
{
   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 depth;
}RvR_ray_pixel_info;

struct RvR_ray_depth_buffer_entry
{
   RvR_fix16 depth;
   int32_t limit;

   RvR_ray_depth_buffer_entry *next;
};

void                  RvR_ray_draw_begin();
void                  RvR_ray_draw_end();
void                  RvR_ray_draw_map(const RvR_ray_cam *cam, const RvR_ray_map *map);
RvR_ray_pixel_info    RvR_ray_map_to_screen(const RvR_ray_cam *cam, RvR_fix16 x, RvR_fix16 y, RvR_fix16 z);

RvR_ray_map *RvR_ray_map_create(uint16_t width, uint16_t height);
void RvR_ray_map_free(RvR_ray_map *map);
RvR_ray_map *RvR_ray_map_load(uint16_t id);
RvR_ray_map *RvR_ray_map_load_path(const char *path);
RvR_ray_map *RvR_ray_map_load_rw(RvR_rw *rw);
void RvR_ray_map_save(const RvR_ray_map *map, const char *path);

void RvR_ray_cast_multi_hit(const RvR_ray_map *map, RvR_ray r, RvR_ray_hit_result *hits, int *hit_count, int max_steps);

int       RvR_ray_map_inbounds(const RvR_ray_map *map, int16_t x, int16_t y);
RvR_fix16 RvR_ray_map_floor_height_at(const RvR_ray_map *map, int16_t x, int16_t y);
RvR_fix16 RvR_ray_map_ceiling_height_at(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_floor_tex_at(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_ceil_tex_at(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ftex_at(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ctex_at(const RvR_ray_map *map, int16_t x, int16_t y);

//TODO: remove?
RvR_fix16 RvR_ray_map_floor_height_at_us(const RvR_ray_map *map, int16_t x, int16_t y);
RvR_fix16 RvR_ray_map_ceiling_height_at_us(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_floor_tex_at_us(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_ceil_tex_at_us(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ftex_at_us(const RvR_ray_map *map, int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ctex_at_us(const RvR_ray_map *map, int16_t x, int16_t y);

void RvR_ray_map_floor_height_set(RvR_ray_map *map, int16_t x, int16_t y, RvR_fix16 height);
void RvR_ray_map_ceiling_height_set(RvR_ray_map *map, int16_t x, int16_t y, RvR_fix16 height);
void RvR_ray_map_floor_tex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_ceil_tex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_wall_ftex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_wall_ctex_set(RvR_ray_map *map, int16_t x, int16_t y, uint16_t tex);

#endif
