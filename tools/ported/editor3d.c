/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "map.h"
#include "texture.h"
#include "color.h"
#include "editor.h"
#include "editor2d.h"
#include "draw.h"
#include "undo.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   STATE3D_VIEW,
   STATE3D_TEX_RECENT,
   STATE3D_TEX_RECENT_GO,
   STATE3D_TEX_ALL,
   STATE3D_TEX_ALL_GO,
}State3D;
//-------------------------------------

//Variables
static int16_t wx = 0;
static int16_t wy = 0;
static int wlocation = 0;

static uint16_t texture_selected = 0;
static int texture_selection_scroll = 0;
static int brush = 0;
static char menu_input[512] = {0};

static int painting = 0;
static int camera_mode = 0;

static State3D state = STATE3D_VIEW;
static RvR_port_selection world_selection;
//-------------------------------------

//Function prototypes
static void e3d_update_view(void);
static void e3d_draw_view(void);
static void e3d_update_tex_recent(void);
static void e3d_draw_tex_recent(void);
static void e3d_update_tex_recent_go(void);
static void e3d_draw_tex_recent_go(void);
static void e3d_update_tex_all(void);
static void e3d_draw_tex_all(void);
static void e3d_update_tex_all_go(void);
static void e3d_draw_tex_all_go(void);
//-------------------------------------

//Function implementations

void editor3d_update(void)
{
   switch(state)
   {
   case STATE3D_VIEW: e3d_update_view(); break;
   case STATE3D_TEX_RECENT: e3d_update_tex_recent(); break;
   case STATE3D_TEX_RECENT_GO: e3d_update_tex_recent_go(); break;
   case STATE3D_TEX_ALL: e3d_update_tex_all(); break;
   case STATE3D_TEX_ALL_GO: e3d_update_tex_all_go(); break;
   }
}

void editor3d_draw(void)
{
   switch(state)
   {
   case STATE3D_VIEW: e3d_draw_view(); break;
   case STATE3D_TEX_RECENT: e3d_draw_tex_recent(); break;
   case STATE3D_TEX_RECENT_GO: e3d_draw_tex_recent_go(); break;
   case STATE3D_TEX_ALL: e3d_draw_tex_all(); break;
   case STATE3D_TEX_ALL_GO: e3d_draw_tex_all_go(); break;
   }

   //Only allow undo in the default state
   switch(state)
   {
   case STATE3D_VIEW:
      if(RvR_key_pressed(RVR_KEY_U))
         undo();
      if(RvR_key_pressed(RVR_KEY_R)&&RvR_key_down(RVR_KEY_LCTRL))
         redo();
      break;
   default:
      break;
   }
}

static void e3d_update_view(void)
{
   if(camera_mode==0)
      camera_update();
   else
      camera_update_collision();
   if(RvR_key_pressed(RVR_KEY_TAB))
      camera_mode = !camera_mode;

   if(RvR_key_pressed(RVR_KEY_V))
   {
      state = STATE3D_TEX_RECENT;
      painting = 0;
      texture_selection_scroll = 0;
      return;
   }

   //Shading
   if(RvR_key_pressed(RVR_KEY_NP_ADD))
   {
      if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
      {
         undo_track_wall_shade(world_selection.as.wall);
         map->walls[world_selection.as.wall].shade_offset++;
      }
      else if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_floor_shade(world_selection.as.sector);
         map->sectors[world_selection.as.sector].shade_floor++;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_ceiling_shade(world_selection.as.sector);
         map->sectors[world_selection.as.sector].shade_ceiling++;
      }
   }
   if(RvR_key_pressed(RVR_KEY_NP_SUB))
   {
      if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
      {
         undo_track_wall_shade(world_selection.as.wall);
         map->walls[world_selection.as.wall].shade_offset--;
      }
      else if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_floor_shade(world_selection.as.sector);
         map->sectors[world_selection.as.sector].shade_floor--;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_ceiling_shade(world_selection.as.sector);
         map->sectors[world_selection.as.sector].shade_ceiling--;
      }
   }

   //Parallax
   if(RvR_key_pressed(RVR_KEY_P))
   {
      if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_flag(world_selection.as.sector);
         map->sectors[world_selection.as.sector].flags ^= RVR_PORT_SECTOR_PARALLAX_FLOOR;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_flag(world_selection.as.sector);
         map->sectors[world_selection.as.sector].flags ^= RVR_PORT_SECTOR_PARALLAX_CEILING;
      }
   }

   //Transparency
   if(RvR_key_pressed(RVR_KEY_T))
   {
      if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_flag(world_selection.as.sprite);
         if(map->sprites[world_selection.as.sprite].flags & RVR_PORT_SPRITE_TRANS0)
            map->sprites[world_selection.as.sprite].flags = (map->sprites[world_selection.as.sprite].flags ^ RVR_PORT_SPRITE_TRANS0) | RVR_PORT_SPRITE_TRANS1;
         else if(map->sprites[world_selection.as.sprite].flags & RVR_PORT_SPRITE_TRANS1)
            map->sprites[world_selection.as.sprite].flags ^= RVR_PORT_SPRITE_TRANS1;
         else
            map->sprites[world_selection.as.sprite].flags |= RVR_PORT_SPRITE_TRANS0;
      }
   }

   //Rotation
   if(RvR_key_pressed(RVR_KEY_PERIOD))
   {
      if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_dir(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].dir -= RvR_key_down(RVR_KEY_LSHIFT)?16:64;
      }
   }
   if(RvR_key_pressed(RVR_KEY_COMMA))
   {
      if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_dir(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].dir += RvR_key_down(RVR_KEY_LSHIFT)?16:64;
      }
   }

   //Sprite placement
   if(RvR_key_pressed(RVR_KEY_S))
   {
      //TODO(Captain4LK): place sprite at location, wall aligned if pointing at walls
   }

   if(RvR_key_pressed(RVR_KEY_F))
   {
      if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
      {
         undo_track_wall_flag(world_selection.as.wall);
         uint16_t wall = world_selection.as.wall;
         uint32_t wflags = map->walls[wall].flags & RVR_PORT_WALL;
         uint32_t one = RVR_PORT_WALL & (-(int32_t)RVR_PORT_WALL);
         map->walls[wall].flags = map->walls[wall].flags & (~RVR_PORT_WALL);
         wflags = (wflags + one) & RVR_PORT_WALL;
         map->walls[wall].flags |= wflags;
      }
      else if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_flag(world_selection.as.sector);
         uint16_t sector = world_selection.as.sector;
         uint32_t fflags = map->sectors[sector].flags & RVR_PORT_SECTOR_FLOOR;
         uint32_t one = RVR_PORT_SECTOR_FLOOR & (-(int32_t)RVR_PORT_SECTOR_FLOOR);
         map->sectors[sector].flags = map->sectors[sector].flags & (~RVR_PORT_SECTOR_FLOOR);
         fflags = (fflags + one) & RVR_PORT_SECTOR_FLOOR;
         map->sectors[sector].flags |= fflags;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_flag(world_selection.as.sector);
         uint16_t sector = world_selection.as.sector;
         uint32_t cflags = map->sectors[sector].flags & RVR_PORT_SECTOR_CEILING;
         uint32_t one = RVR_PORT_SECTOR_CEILING & (-(int32_t)RVR_PORT_SECTOR_CEILING);
         map->sectors[sector].flags = map->sectors[sector].flags & (~RVR_PORT_SECTOR_CEILING);
         cflags = (cflags + one) & RVR_PORT_SECTOR_CEILING;
         map->sectors[sector].flags |= cflags;
      }
   }

   //Texture coordinates
   if(RvR_key_pressed(RVR_KEY_NP4))
   {
      if(RvR_key_down(RVR_KEY_LSHIFT))
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_units(world_selection.as.wall);
            map->walls[world_selection.as.wall].x_units--;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_units(world_selection.as.sector);
            map->sectors[world_selection.as.sector].x_units--;
         }
         else if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
         {
            undo_track_sprite_units(world_selection.as.sprite);
            map->sprites[world_selection.as.sprite].x_units--;
         }
      }
      else
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_offsets(world_selection.as.wall);
            map->walls[world_selection.as.wall].x_off++;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_offsets(world_selection.as.sector);
            map->sectors[world_selection.as.sector].x_off++;
         }
      }
   }
   if(RvR_key_pressed(RVR_KEY_NP6))
   {
      if(RvR_key_down(RVR_KEY_LSHIFT))
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_units(world_selection.as.wall);
            map->walls[world_selection.as.wall].x_units++;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_units(world_selection.as.sector);
            map->sectors[world_selection.as.sector].x_units++;
         }
         else if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
         {
            undo_track_sprite_units(world_selection.as.sprite);
            map->sprites[world_selection.as.sprite].x_units++;
         }
      }
      else
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_offsets(world_selection.as.wall);
            map->walls[world_selection.as.wall].x_off--;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_offsets(world_selection.as.sector);
            map->sectors[world_selection.as.sector].x_off--;
         }
      }
   }
   if(RvR_key_pressed(RVR_KEY_NP8))
   {
      if(RvR_key_down(RVR_KEY_LSHIFT))
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_units(world_selection.as.wall);
            map->walls[world_selection.as.wall].y_units++;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_units(world_selection.as.sector);
            map->sectors[world_selection.as.sector].y_units++;
         }
         else if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
         {
            undo_track_sprite_units(world_selection.as.sprite);
            map->sprites[world_selection.as.sprite].y_units++;
         }
      }
      else
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_offsets(world_selection.as.wall);
            map->walls[world_selection.as.wall].y_off++;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_offsets(world_selection.as.sector);
            map->sectors[world_selection.as.sector].y_off--;
         }
      }
   }
   if(RvR_key_pressed(RVR_KEY_NP2))
   {
      if(RvR_key_down(RVR_KEY_LSHIFT))
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_units(world_selection.as.wall);
            map->walls[world_selection.as.wall].y_units--;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_units(world_selection.as.sector);
            map->sectors[world_selection.as.sector].y_units--;
         }
         else if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
         {
            undo_track_sprite_units(world_selection.as.sprite);
            map->sprites[world_selection.as.sprite].y_units--;
         }
      }
      else
      {
         if(world_selection.type==RVR_PORT_SWALL_BOT||world_selection.type==RVR_PORT_SWALL_TOP)
         {
            undo_track_wall_offsets(world_selection.as.wall);
            map->walls[world_selection.as.wall].y_off--;
         }
         else if(world_selection.type==RVR_PORT_SFLOOR||world_selection.type==RVR_PORT_SCEILING)
         {
            undo_track_sector_offsets(world_selection.as.sector);
            map->sectors[world_selection.as.sector].y_off++;
         }
      }
   }

   //Floor/Ceiling align
   if(RvR_key_pressed(RVR_KEY_R)&&!RvR_key_down(RVR_KEY_LCTRL))
   {
      if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_flag(world_selection.as.sector);
         map->sectors[world_selection.as.sector].flags ^= RVR_PORT_SECTOR_ALIGN_FLOOR;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_flag(world_selection.as.sector);
         map->sectors[world_selection.as.sector].flags ^= RVR_PORT_SECTOR_ALIGN_CEILING;
      }
      else if(world_selection.type==RVR_PORT_SSPRITE_BILL)
      {
         undo_track_sprite_flag(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].flags |= RVR_PORT_SPRITE_WALL;
      }
      else if(world_selection.type==RVR_PORT_SSPRITE_WALL)
      {
         undo_track_sprite_flag(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].flags = (map->sprites[world_selection.as.sprite].flags ^ RVR_PORT_SPRITE_WALL) | RVR_PORT_SPRITE_FLOOR;
      }
      else if(world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_flag(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].flags ^= RVR_PORT_SPRITE_FLOOR;
      }
   }

   //Slope
   if(RvR_key_pressed(RVR_KEY_8))
   {
      int count = RvR_key_down(RVR_KEY_LSHIFT)?1:16;
      if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_slope(world_selection.as.sector);
         map->sectors[world_selection.as.sector].slope_floor -= count;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_slope(world_selection.as.sector);
         map->sectors[world_selection.as.sector].slope_ceiling -= count;
      }
   }
   if(RvR_key_pressed(RVR_KEY_9))
   {
      int count = RvR_key_down(RVR_KEY_LSHIFT)?1:16;
      if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_slope(world_selection.as.sector);
         map->sectors[world_selection.as.sector].slope_floor += count;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_slope(world_selection.as.sector);
         map->sectors[world_selection.as.sector].slope_ceiling += count;
      }
   }

   //Floor/Ceiling/Sprite height
   if(RvR_key_pressed(RVR_KEY_PGUP))
   {
      if(world_selection.type==RVR_PORT_SWALL_BOT)
      {
         int16_t sector = RvR_port_wall_sector(map, world_selection.as.wall);
         if(map->walls[world_selection.as.wall].portal!=RVR_PORT_SECTOR_INVALID)
            sector = map->walls[world_selection.as.wall].portal;

         undo_track_sector_height(sector);
         map->sectors[sector].floor += 128;
      }
      else if(world_selection.type==RVR_PORT_SWALL_TOP)
      {
         int16_t sector = RvR_port_wall_sector(map, world_selection.as.wall);
         if(map->walls[world_selection.as.wall].portal!=RVR_PORT_SECTOR_INVALID)
            sector = map->walls[world_selection.as.wall].portal;

         undo_track_sector_height(sector);
         map->sectors[sector].ceiling += 128;
      }
      else if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_height(world_selection.as.sector);
         map->sectors[world_selection.as.sector].floor += 128;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_height(world_selection.as.sector);
         map->sectors[world_selection.as.sector].ceiling += 128;
      }
      else if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_pos(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].z += 128;
      }
   }
   else if(RvR_key_pressed(RVR_KEY_PGDN))
   {
      if(world_selection.type==RVR_PORT_SWALL_BOT)
      {
         int16_t sector = RvR_port_wall_sector(map, world_selection.as.wall);
         if(map->walls[world_selection.as.wall].portal!=RVR_PORT_SECTOR_INVALID)
            sector = map->walls[world_selection.as.wall].portal;

         undo_track_sector_height(sector);
         map->sectors[sector].floor -= 128;
      }
      else if(world_selection.type==RVR_PORT_SWALL_TOP)
      {
         int16_t sector = RvR_port_wall_sector(map, world_selection.as.wall);
         if(map->walls[world_selection.as.wall].portal!=RVR_PORT_SECTOR_INVALID)
            sector = map->walls[world_selection.as.wall].portal;

         undo_track_sector_height(sector);
         map->sectors[sector].ceiling -= 128;
      }
      else if(world_selection.type==RVR_PORT_SFLOOR)
      {
         undo_track_sector_height(world_selection.as.sector);
         map->sectors[world_selection.as.sector].floor -= 128;
      }
      else if(world_selection.type==RVR_PORT_SCEILING)
      {
         undo_track_sector_height(world_selection.as.sector);
         map->sectors[world_selection.as.sector].ceiling -= 128;
      }
      else if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_pos(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].z -= 128;
      }
   }

   if(RvR_key_pressed(RVR_BUTTON_LEFT))
   {
      if(world_selection.type==RVR_PORT_SSPRITE_BILL||world_selection.type==RVR_PORT_SSPRITE_WALL||world_selection.type==RVR_PORT_SSPRITE_FLOOR)
      {
         undo_track_sprite_tex(world_selection.as.sprite);
         map->sprites[world_selection.as.sprite].tex = texture_selected;
      }
      else
      {
         painting = 1;
      }
   }
   if(RvR_key_released(RVR_BUTTON_LEFT))
      painting = 0;

   //TODO(Captain4LK): maybe continously track all changes and only write one undo entry while holding done?
   if(painting)
   {
      if(world_selection.type==RVR_PORT_SWALL_BOT&&map->walls[world_selection.as.wall].tex_lower!=texture_selected)
      {
         undo_track_wall_tex(world_selection.as.wall);
         map->walls[world_selection.as.wall].tex_lower = texture_selected;
      }
      else if(world_selection.type==RVR_PORT_SWALL_TOP&&map->walls[world_selection.as.wall].tex_upper!=texture_selected)
      {
         undo_track_wall_tex(world_selection.as.wall);
         map->walls[world_selection.as.wall].tex_upper = texture_selected;
      }
      else if(world_selection.type==RVR_PORT_SFLOOR&&map->sectors[world_selection.as.sector].floor_tex!=texture_selected)
      {
         undo_track_sector_tex(world_selection.as.sector);
         map->sectors[world_selection.as.sector].floor_tex = texture_selected;
      }
      else if(world_selection.type==RVR_PORT_SCEILING&&map->sectors[world_selection.as.sector].ceiling_tex!=texture_selected)
      {
         undo_track_sector_tex(world_selection.as.sector);
         map->sectors[world_selection.as.sector].ceiling_tex = texture_selected;
      }
   }

   if(RvR_key_pressed(RVR_KEY_ENTER))
      editor_set_2d();
}

static void e3d_draw_view(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   RvR_port_draw_begin(map, &camera);

   world_selection.x = mx;
   world_selection.y = my;
   RvR_port_draw_map(&world_selection);

   for(int i = 0; i<map->sprite_count; i++)
      RvR_port_draw_sprite(map->sprites+i, NULL);

   RvR_port_draw_end(&world_selection);

   RvR_port_report report;
   RvR_port_draw_report(&report);
   //printf("Report: swaps: %d; stack_max: %d\n",report.sort_swaps,report.stack_max);

   RvR_render_rectangle(8, RvR_yres() - 74, 66, 66, color_white);
   draw_fit64(9, RvR_yres() - 73, texture_selected);

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}

static void e3d_update_tex_recent(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_pressed(RVR_KEY_V))
   {
      state = STATE3D_TEX_ALL;
      texture_selection_scroll = 0;
   }

   if(RvR_key_pressed(RVR_KEY_ESCAPE))
      state = STATE3D_VIEW;

   if(RvR_key_pressed(RVR_KEY_G))
   {
      state = STATE3D_TEX_RECENT_GO;
      menu_input[0] = '\0';
      RvR_text_input_start(menu_input, 64);
   }

   texture_selection_scroll += RvR_mouse_wheel_scroll() * -3;
   if(texture_selection_scroll<0)
      texture_selection_scroll = 0;

   int tex_per_row = (RvR_xres()) / 64;
   int tex_per_col = (RvR_yres()) / 64;
   int index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
   int set_pos = 0;
   if(RvR_key_pressed(RVR_KEY_UP))
   {
      index -= tex_per_row;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
      if(my<0)
      {
         texture_selection_scroll--;
         my += 64;
      }

      set_pos = 1;
   }
   if(RvR_key_pressed(RVR_KEY_DOWN))
   {
      index += tex_per_row;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
      if(my>tex_per_col * 64)
      {
         texture_selection_scroll++;
         my -= 64;
      }

      set_pos = 1;
   }
   if(RvR_key_pressed(RVR_KEY_LEFT))
   {
      index--;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;

      set_pos = 1;
   }
   if(RvR_key_pressed(RVR_KEY_RIGHT))
   {
      index++;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;

      set_pos = 1;
   }

   if(index<0)
   {
      texture_selection_scroll = 0;
      RvR_mouse_set_pos(32, 32);
      mx = 32;
      my = 32;
      index = 0;
      set_pos = 0;
   }

   if(set_pos)
      RvR_mouse_set_pos(mx, my);

   if(mx / 64<RvR_xres() / 64)
   {
      if(RvR_key_pressed(RVR_BUTTON_LEFT)||RvR_key_pressed(RVR_KEY_ENTER))
      {
         index = texture_list_used_wrap(texture_list_used.data_last - (mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64));
         texture_selected = texture_list_used.data[index];
         texture_list_used_add(texture_selected);
         state = STATE3D_VIEW;
         brush = 0;
      }
   }
}

static void e3d_draw_tex_recent(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   RvR_render_clear(color_black);

   for(int y = 0; y<=RvR_yres() / 64; y++)
   {
      for(int x = 0; x<=RvR_xres() / 64; x++)
      {
         int index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
         index = texture_list_used_wrap(texture_list_used.data_last - index);
         draw_fit64(x * 64, y * 64, texture_list_used.data[index]);

         RvR_render_font_set(0xF001);
         char tmp_font[16];
         snprintf(tmp_font, 16, "%d", texture_list_used.data[index]);
         RvR_render_rectangle_fill(x * 64, y * 64, (int)strlen(tmp_font) * 4 + 1, 7, color_black);
         RvR_render_string(x * 64 + 1, y * 64 + 1, 1, tmp_font, color_yellow);
         RvR_render_font_set(0xF000);
      }
   }

   if(mx / 64<RvR_xres() / 64)
      RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}

static void e3d_update_tex_recent_go(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_pressed(RVR_KEY_ENTER))
   {
      RvR_text_input_end();
      state = STATE3D_TEX_ALL;

      int selection = (int)strtol(menu_input, NULL, 10);
      //TODO: What we should do: binary search
      //What I did: slow crap
      for(int i = 0; i<texture_list.data_used; i++)
      {
         if(texture_list.data[i]>=selection)
         {
            int tex_per_row = (RvR_xres()) / 64;
            texture_selection_scroll = i / RvR_non_zero(tex_per_row);
            mx = (i % tex_per_row) * 64 + 32;
            my = (i / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
            RvR_mouse_set_pos(mx, my);
            break;
         }
      }
   }
}

static void e3d_draw_tex_recent_go(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   RvR_render_clear(color_black);

   for(int y = 0; y<=RvR_yres() / 64; y++)
   {
      for(int x = 0; x<=RvR_xres() / 64; x++)
      {
         int index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
         index = texture_list_used_wrap(texture_list_used.data_last - index);
         draw_fit64(x * 64, y * 64, texture_list_used.data[index]);

         RvR_render_font_set(0xF001);
         char tmp_font[16];
         snprintf(tmp_font, 16, "%d", texture_list_used.data[index]);
         RvR_render_rectangle_fill(x * 64, y * 64, (int)strlen(tmp_font) * 4 + 1, 7, color_black);
         RvR_render_string(x * 64 + 1, y * 64 + 1, 1, tmp_font, color_yellow);
         RvR_render_font_set(0xF000);
      }
   }

   if(mx / 64<RvR_xres() / 64)
      RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);

   RvR_render_rectangle_fill(0, 0, RvR_xres(), 12, color_dark_gray);
   RvR_render_string(2, 2, 1, "Go to: ", color_white);
   RvR_render_string(35, 2, 1, menu_input, color_white);

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}

static void e3d_update_tex_all(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_pressed(RVR_KEY_ESCAPE))
      state = STATE3D_VIEW;

   if(RvR_key_pressed(RVR_KEY_G))
   {
      state = STATE3D_TEX_ALL_GO;
      menu_input[0] = '\0';
      RvR_text_input_start(menu_input, 64);
   }

   texture_selection_scroll += RvR_mouse_wheel_scroll() * -3;
   if(texture_selection_scroll<0)
      texture_selection_scroll = 0;

   int tex_per_row = (RvR_xres()) / 64;
   int tex_per_col = (RvR_yres()) / 64;
   int index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
   int set_pos = 0;
   if(RvR_key_pressed(RVR_KEY_UP))
   {
      index -= tex_per_row;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
      if(my<0)
      {
         texture_selection_scroll--;
         my += 64;
      }

      set_pos = 1;
   }
   if(RvR_key_pressed(RVR_KEY_DOWN))
   {
      index += tex_per_row;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
      if(my>tex_per_col * 64)
      {
         texture_selection_scroll++;
         my -= 64;
      }

      set_pos = 1;
   }
   if(RvR_key_pressed(RVR_KEY_LEFT))
   {
      index--;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;

      set_pos = 1;
   }
   if(RvR_key_pressed(RVR_KEY_RIGHT))
   {
      index++;
      mx = (index % tex_per_row) * 64 + 32;
      my = (index / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;

      set_pos = 1;
   }

   if(index<0)
   {
      texture_selection_scroll = 0;
      RvR_mouse_set_pos(32, 32);
      mx = 32;
      my = 32;
      index = 0;
      set_pos = 0;
   }

   if(set_pos)
      RvR_mouse_set_pos(mx, my);

   if(mx / 64<RvR_xres() / 64)
   {
      if(RvR_key_pressed(RVR_BUTTON_LEFT)||RvR_key_pressed(RVR_KEY_ENTER))
      {
         index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
         if(index<texture_list.data_used)
         {
            texture_selected = texture_list.data[index];
            texture_list_used_add(texture_selected);
            state = STATE3D_VIEW;
            brush = 0;
         }
      }
   }
}

static void e3d_draw_tex_all(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   RvR_render_clear(color_black);

   for(int y = 0; y<=RvR_yres() / 64; y++)
   {
      for(int x = 0; x<=RvR_xres() / 64; x++)
      {
         unsigned index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
         if(index<texture_list.data_used)
         {
            draw_fit64(x * 64, y * 64, texture_list.data[index]);

            RvR_render_font_set(0xF001);
            char tmp_font[16];
            snprintf(tmp_font, 16, "%d", texture_list.data[index]);
            RvR_render_rectangle_fill(x * 64, y * 64, (int)strlen(tmp_font) * 4 + 1, 7, color_black);
            RvR_render_string(x * 64 + 1, y * 64 + 1, 1, tmp_font, color_yellow);
            RvR_render_font_set(0xF000);
         }
      }
   }

   if(mx / 64<RvR_xres() / 64)
      RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}

static void e3d_update_tex_all_go(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_pressed(RVR_KEY_ENTER))
   {
      RvR_text_input_end();
      state = STATE3D_TEX_ALL;

      int selection = (int)strtol(menu_input, NULL, 10);
      //TODO: What we should do: binary search
      //What I did: slow crap
      for(int i = 0; i<texture_list.data_used; i++)
      {
         if(texture_list.data[i]>=selection)
         {
            int tex_per_row = (RvR_xres()) / 64;
            texture_selection_scroll = i / RvR_non_zero(tex_per_row);
            mx = (i % tex_per_row) * 64 + 32;
            my = (i / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
            RvR_mouse_set_pos(mx, my);
            break;
         }
      }
   }
}

static void e3d_draw_tex_all_go(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   RvR_render_clear(color_black);

   for(int y = 0; y<=RvR_yres() / 64; y++)
   {
      for(int x = 0; x<=RvR_xres() / 64; x++)
      {
         unsigned index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
         if(index<texture_list.data_used)
         {
            draw_fit64(x * 64, y * 64, texture_list.data[index]);

            RvR_render_font_set(0xF001);
            char tmp_font[16];
            snprintf(tmp_font, 16, "%d", texture_list.data[index]);
            RvR_render_rectangle_fill(x * 64, y * 64, (int)strlen(tmp_font) * 4 + 1, 7, color_black);
            RvR_render_string(x * 64 + 1, y * 64 + 1, 1, tmp_font, color_yellow);
            RvR_render_font_set(0xF000);
         }
      }
   }

   if(mx / 64<RvR_xres() / 64)
      RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);

   RvR_render_rectangle_fill(0, 0, RvR_xres(), 12, color_dark_gray);
   RvR_render_string(2, 2, 1, "Go to: ", color_white);
   RvR_render_string(35, 2, 1, menu_input, color_white);

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}
//-------------------------------------
