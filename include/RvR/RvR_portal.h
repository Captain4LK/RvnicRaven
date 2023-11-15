/*
RvnicRaven - Portal renderer

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PORTAL_H_

#define _RVR_PORTAL_H_

typedef struct RvR_port_depth_buffer_entry RvR_port_depth_buffer_entry;

typedef struct
{
   int16_t wall_count;
   int16_t wall_first;
   RvR_fix22 floor;
   RvR_fix22 ceiling;
   uint16_t floor_tex;
   uint16_t ceiling_tex;
   uint8_t visited;
}RvR_port_sector;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   uint32_t flags;
   int16_t p2;
   int16_t portal;
   int16_t join;
   uint16_t tex;
}RvR_port_wall;

typedef struct
{
   RvR_port_wall *walls;
   RvR_port_sector *sectors;
   int32_t wall_count;
   int32_t sector_count;
}RvR_port_map;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
   RvR_fix22 dir;
   RvR_fix22 fov;
   RvR_fix22 shear;
   int16_t sector;
}RvR_port_cam;

typedef enum
{
   RVR_PORT_NONE,
   RVR_PORT_WALL,
   RVR_PORT_SECTOR,
   RVR_PORT_SPRITE_BILL,
   RVR_PORT_SPRITE_WALL,
   RVR_PORT_SPRITE_FLOOR,
}RvR_port_select;

typedef struct
{
   int x;
   int y;
   RvR_fix22 depth;
   RvR_port_select type;
   union
   {
      uint32_t wall;
      uint32_t sector;
   }as;
}RvR_port_selection;

struct RvR_port_depth_buffer_entry
{
   RvR_fix22 depth;
   int32_t limit;

   RvR_port_depth_buffer_entry *next;
};

RvR_port_map *RvR_port_map_create(void);
void RvR_port_map_save(const RvR_port_map *map, const char *path);
RvR_port_map *RvR_port_map_load(uint16_t id);
RvR_port_map *RvR_port_map_load_path(const char *path);
RvR_port_map *RvR_port_map_load_rw(RvR_rw *rw);

int RvR_port_sector_inside(const RvR_port_map *map, int16_t sector, RvR_fix22 x, RvR_fix22 y);
int16_t RvR_port_sector_update(const RvR_port_map *map, int16_t sector_last, RvR_fix22 x, RvR_fix22 y);
int16_t RvR_port_sector_new(RvR_port_map *map, RvR_fix22 x, RvR_fix22 y);
void RvR_port_sector_fix_winding(RvR_port_map *map, int16_t sector);
int16_t RvR_port_sector_make_inner(RvR_port_map *map, int16_t wall);

int16_t RvR_port_wall_sector(const RvR_port_map *map, int16_t wall);
void RvR_port_wall_move(RvR_port_map *map, int16_t wall, RvR_fix22 x, RvR_fix22 y);
//Finds first wall in polygon
int16_t RvR_port_wall_first(const RvR_port_map *map, int16_t wall);
//For adding a point to an UNFINISHED polygon or creating a new polygon
int16_t RvR_port_wall_append(RvR_port_map *map, int16_t sector, RvR_fix22 x, RvR_fix22 y);
//For subdividing lines in COMPLETED polygins
int16_t RvR_port_wall_insert(RvR_port_map *map, int16_t w0, RvR_fix22 x, RvR_fix22 y);
int16_t RvR_port_wall_next(const RvR_port_map *map, int16_t wall);
int16_t RvR_port_wall_previous(const RvR_port_map *map, int16_t wall);

void RvR_port_draw_begin(const RvR_port_map *map, const RvR_port_cam *cam);
void RvR_port_draw_map(RvR_port_selection *select);
void RvR_port_draw_end(RvR_port_selection *select);
void RvR_port_draw_sprite(RvR_fix22 x, RvR_fix22 y, RvR_fix22 z, RvR_fix22 dir, int16_t sector, uint16_t sprite, uint32_t flags, void *ref);

const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_floor(int x);
const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_ceiling(int x);

#endif
