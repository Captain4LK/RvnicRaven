/*
RvnicRaven - Portal renderer

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PORTAL_H_

#define _RVR_PORTAL_H_

typedef struct RvR_port_depth_buffer_entry RvR_port_depth_buffer_entry;

typedef struct
{
   uint16_t wall_count;
   uint16_t wall_first;
   uint32_t flags;
   RvR_fix22 floor;
   RvR_fix22 ceiling;
   uint16_t floor_tex;
   uint16_t ceiling_tex;
   uint8_t visited;
   int8_t shade_floor;
   int8_t shade_ceiling;
   int16_t slope_floor;
   int16_t slope_ceiling;
   int16_t x_off;
   int16_t y_off;
   uint8_t x_units;
   uint8_t y_units;
}RvR_port_sector;

//TODO(Captain4LK): maybe replace x,y position with
//separate vertex array indices. This would make moving walls trivial
//(since all connected walls share vertices).
//But might reduce performance due to worse cache access
typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   uint32_t flags;
   uint16_t p2;
   uint16_t portal;
   uint16_t portal_wall;
   uint16_t tex_lower;
   uint16_t tex_upper;
   uint16_t tex_mid;
   int8_t shade_offset;
   int16_t x_off;
   int16_t y_off;
   uint8_t x_units;
   uint8_t y_units;
}RvR_port_wall;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
   RvR_fix22 dir;
   uint16_t sector;
   uint16_t tex;
   uint32_t flags;
   uint8_t x_units;
   uint8_t y_units;
}RvR_port_sprite;

typedef struct
{
   RvR_port_wall *walls;
   RvR_port_sector *sectors;
   RvR_port_sprite *sprites;
   uint16_t wall_count;
   uint16_t sector_count;
   uint16_t sprite_count;
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
   RVR_PORT_SNONE,
   RVR_PORT_SWALL_TOP,
   RVR_PORT_SWALL_BOT,
   RVR_PORT_SFLOOR,
   RVR_PORT_SCEILING,
   RVR_PORT_SSPRITE_BILL,
   RVR_PORT_SSPRITE_WALL,
   RVR_PORT_SSPRITE_FLOOR,
}RvR_port_select;

typedef struct
{
   int x;
   int y;
   RvR_fix22 depth;
   RvR_port_select type;

   //Texture coordinates
   int tx;
   int ty;

   union
   {
      uint16_t wall;
      uint16_t sector;
      uint16_t sprite;
   }as;
}RvR_port_selection;

struct RvR_port_depth_buffer_entry
{
   RvR_fix22 depth;
   int32_t limit;

   RvR_port_depth_buffer_entry *next;
};

//Render report
typedef struct
{
   int stack_max;
   int sort_swaps;
   int sort_skips;
}RvR_port_report;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
   RvR_fix22 ox;
   RvR_fix22 oy;
   RvR_fix22 oz;
   //RvR_fix22 d;
}RvR_port_slope;

//Sector flags
//-------------------------------------
#define RVR_PORT_SECTOR_PARALLAX_FLOOR           UINT32_C(0x1)
#define RVR_PORT_SECTOR_PARALLAX_CEILING         UINT32_C(0x2)
#define RVR_PORT_SECTOR_ALIGN_FLOOR              UINT32_C(0x4)
#define RVR_PORT_SECTOR_ALIGN_CEILING            UINT32_C(0x8)

#define RVR_PORT_SECTOR_FLIP_X_FLOOR             UINT32_C(0x10)
#define RVR_PORT_SECTOR_FLIP_Y_FLOOR             UINT32_C(0x20)
#define RVR_PORT_SECTOR_ROT_FLOOR                UINT32_C(0x40)
#define RVR_PORT_SECTOR_FLOOR                    (RVR_PORT_SECTOR_FLIP_X_FLOOR | RVR_PORT_SECTOR_FLIP_Y_FLOOR | RVR_PORT_SECTOR_ROT_FLOOR)

#define RVR_PORT_SECTOR_FLIP_X_CEILING           UINT32_C(0x80)
#define RVR_PORT_SECTOR_FLIP_Y_CEILING           UINT32_C(0x100)
#define RVR_PORT_SECTOR_ROT_CEILING              UINT32_C(0x200)
#define RVR_PORT_SECTOR_CEILING                  (RVR_PORT_SECTOR_FLIP_X_CEILING | RVR_PORT_SECTOR_FLIP_Y_CEILING | RVR_PORT_SECTOR_ROT_CEILING)
//-------------------------------------

//Wall flags
//-------------------------------------
#define RVR_PORT_WALL_FLIP_X                     UINT32_C(0x1)
#define RVR_PORT_WALL_FLIP_Y                     UINT32_C(0x2)
#define RVR_PORT_WALL                            (RVR_PORT_WALL_FLIP_X | RVR_PORT_WALL_FLIP_Y)
//-------------------------------------

//Sprite flags
//-------------------------------------
#define RVR_PORT_SPRITE_INVISIBLE                UINT32_C(0x1)
#define RVR_PORT_SPRITE_WALL                     UINT32_C(0x2)
#define RVR_PORT_SPRITE_FLOOR                    UINT32_C(0x4)
#define RVR_PORT_SPRITE_ONESIDED                 UINT32_C(0x8)
#define RVR_PORT_SPRITE_XFLIP                    UINT32_C(0x10)
#define RVR_PORT_SPRITE_YFLIP                    UINT32_C(0x20)
#define RVR_PORT_SPRITE_TRANS0                   UINT32_C(0x40)
#define RVR_PORT_SPRITE_TRANS1                   UINT32_C(0x80)
#define RVR_PORT_SPRITE_CENTER                   UINT32_C(0x100)
//-------------------------------------

//Constants
#define RVR_PORT_WALL_INVALID UINT16_MAX
#define RVR_PORT_SECTOR_INVALID UINT16_MAX
#define RVR_PORT_SPRITE_INVALID UINT16_MAX
//-------------------------------------

//Map
//-------------------------------------
RvR_port_map *RvR_port_map_create(void);
void          RvR_port_map_save(const RvR_port_map *map, const char *path);
RvR_port_map *RvR_port_map_load(uint16_t id);
RvR_port_map *RvR_port_map_load_path(const char *path);
RvR_port_map *RvR_port_map_load_rw(RvR_rw *rw);
int           RvR_port_map_check(const RvR_port_map *map);
void          RvR_port_map_print_walls(const RvR_port_map *map);
//-------------------------------------

//Sector
//-------------------------------------
int     RvR_port_sector_inside(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y);
uint16_t RvR_port_sector_update(const RvR_port_map *map, uint16_t sector_last, RvR_fix22 x, RvR_fix22 y);
RvR_fix22 RvR_port_sector_floor_at(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y);
RvR_fix22 RvR_port_sector_ceiling_at(const RvR_port_map *map, uint16_t sector, RvR_fix22 x, RvR_fix22 y);

//Map manipulation
//TODO(Captain4LK): should we keep these here, or should they be only in the editor?
void     RvR_port_sector_fix_winding(RvR_port_map *map, uint16_t sector);
uint16_t RvR_port_sector_make_inner(RvR_port_map *map, uint16_t wall);
void     RvR_port_sector_delete(RvR_port_map *map, uint16_t sector);
uint16_t RvR_port_sector_join(RvR_port_map *map, uint16_t sector0, uint16_t sector1);
//-------------------------------------

//Wall
//-------------------------------------
uint16_t RvR_port_wall_sector(const RvR_port_map *map, uint16_t wall);
void    RvR_port_wall_move(RvR_port_map *map, uint16_t wall, RvR_fix22 x, RvR_fix22 y);
uint16_t RvR_port_wall_make_first(RvR_port_map *map, uint16_t wall);
uint16_t RvR_port_wall_first(const RvR_port_map *map, uint16_t wall);
uint16_t RvR_port_wall_next(const RvR_port_map *map, uint16_t wall);
uint16_t RvR_port_wall_previous(const RvR_port_map *map, uint16_t wall);
int RvR_port_wall_winding(const RvR_port_map *map, uint16_t wall);
int RvR_port_wall_inside(const RvR_port_map *map, uint16_t wall, RvR_fix22 x, RvR_fix22 y);
int     RvR_port_wall_subsector(const RvR_port_map *map, uint16_t sector, uint16_t wall);
uint16_t RvR_port_subsector_length(const RvR_port_map *map, uint16_t wall);
uint16_t RvR_port_subsector_first(const RvR_port_map *map, uint16_t wall);
void     RvR_port_wall_delete(RvR_port_map *map, uint16_t wall);
//TODO(Captain4LK): find a proper name for this
uint16_t RvR_port_wall_next_onesided(const RvR_port_map *map, uint16_t wall);
uint16_t RvR_port_wall_onesided_length(const RvR_port_map *map, uint16_t wall);

//Map manipulation
//TODO(Captain4LK): should we keep these here, or should they be only in the editor?
uint16_t RvR_port_wall_insert(RvR_port_map *map, uint16_t w0, RvR_fix22 x, RvR_fix22 y);
//-------------------------------------

//Slopes
//-------------------------------------
void RvR_port_slope_from_floor(const RvR_port_map *map, uint16_t sector, RvR_port_slope *slope);
void RvR_port_slope_from_ceiling(const RvR_port_map *map, uint16_t sector, RvR_port_slope *slope);
RvR_fix22 RvR_port_slope_height_at(const RvR_port_slope *slope, RvR_fix22 x, RvR_fix22 y);
//-------------------------------------

//Rendering
//-------------------------------------
void  RvR_port_draw_begin(const RvR_port_map *map, const RvR_port_cam *cam);
void  RvR_port_draw_map(RvR_port_selection *select);
void  RvR_port_draw_end(RvR_port_selection *select);
void  RvR_port_draw_sprite(RvR_port_sprite *spr, void *ref);
void  RvR_port_draw_report(RvR_port_report *report);

const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_floor(int x);
const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_ceiling(int x);
//-------------------------------------


#endif
