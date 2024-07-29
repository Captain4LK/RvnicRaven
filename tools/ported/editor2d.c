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
#include <inttypes.h>

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "color.h"
#include "map.h"
#include "undo.h"
#include "editor.h"
#include "editor2d.h"
#include "sector_draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   STATE2D_VIEW,
   STATE2D_VIEW_SCROLL,
   STATE2D_WALL_MOVE,
   STATE2D_SPRITE_MOVE,
   STATE2D_SECTOR,
}State2D;
//-------------------------------------

//Variables
static int scroll_x = 0;
static int scroll_y = 0;
static int mouse_scroll = 0;
static char menu_input[512] = {0};

static uint16_t sprite_sel = RVR_PORT_SPRITE_INVALID;
static uint16_t sprite_move = RVR_PORT_SPRITE_INVALID;

static uint16_t wall_move = RVR_PORT_WALL_INVALID;
static RvR_fix22 wall_move_x = 0;
static RvR_fix22 wall_move_y = 0;
static uint16_t hover = RVR_PORT_WALL_INVALID;

static State2D state = STATE2D_VIEW;

static RvR_fix22 world_mx;
static RvR_fix22 world_my;

static RvR_fix22 zoom = 32;
static int draw_grid = 0;
static const RvR_fix22 draw_grid_sizes[8] = { 10, 9, 8, 7, 6, 5, 4, 0, };

static uint16_t sector_join = RVR_PORT_SECTOR_INVALID;
//-------------------------------------

//Function prototypes
static void e2d_draw_base(void);

static void e2d_update_view(void);
static void e2d_draw_view(void);
static void e2d_update_view_scroll(void);
static void e2d_draw_view_scroll(void);
static void e2d_update_wall_move(void);
static void e2d_draw_wall_move(void);
static void e2d_update_sprite_move(void);
static void e2d_draw_sprite_move(void);
static void e2d_update_sector(void);
static void e2d_draw_sector(void);
//-------------------------------------

//Function implementations

void editor2d_update(void)
{
   switch(state)
   {
   case STATE2D_VIEW: e2d_update_view(); break;
   case STATE2D_VIEW_SCROLL: e2d_update_view_scroll(); break;
   case STATE2D_WALL_MOVE: e2d_update_wall_move(); break;
   case STATE2D_SPRITE_MOVE: e2d_update_sprite_move(); break;
   case STATE2D_SECTOR: e2d_update_sector(); break;
   }
}

void editor2d_draw(void)
{
   switch(state)
   {
   case STATE2D_VIEW: e2d_draw_view(); break;
   case STATE2D_VIEW_SCROLL: e2d_draw_view_scroll(); break;
   case STATE2D_WALL_MOVE: e2d_draw_wall_move(); break;
   case STATE2D_SPRITE_MOVE: e2d_draw_sprite_move(); break;
   case STATE2D_SECTOR: e2d_draw_sector(); break;
   }
}

static void e2d_draw_base(void)
{
   RvR_render_clear(color_black);


   //Draw grid
   if(draw_grid_sizes[draw_grid]>0&&(RvR_yres() * zoom) / ((1 << draw_grid_sizes[draw_grid]))<=RvR_yres() / 2)
   {
      int dgrid = 1 << draw_grid_sizes[draw_grid];
      int start = -((RvR_yres() / 2) * zoom) / RvR_non_zero(dgrid);
      int end = +((RvR_yres() / 2) * zoom) / RvR_non_zero(dgrid);
      for(int y = start; y<=end; y++)
      {
         RvR_fix22 wy = -(camera.y % dgrid) + y * dgrid + (camera.y);
         RvR_render_horizontal_line(0, RvR_xres(), (wy - camera.y) / RvR_non_zero(zoom) + RvR_yres() / 2, color_dark_gray);
      }

      start = -((RvR_xres() / 2) * zoom) / RvR_non_zero(dgrid);
      end = +((RvR_xres() / 2) * zoom) / RvR_non_zero(dgrid);
      for(int x = start; x<=end; x++)
      {
         RvR_fix22 wx = -(camera.x % dgrid) + x * dgrid + (camera.x);
         RvR_render_vertical_line((wx - camera.x) / RvR_non_zero(zoom) + RvR_xres() / 2, 0, RvR_yres(), color_dark_gray);
      }
   }

   //Draw sectors
   for(int i = 0; i<map->sector_count; i++)
   {
      for(int j = 0; j<map->sectors[i].wall_count; j++)
      {
         RvR_port_wall *p0 = map->walls + map->sectors[i].wall_first + j;
         RvR_port_wall *p1 = map->walls + p0->p2;

         if(p0->p2==RVR_PORT_WALL_INVALID)
            continue;

         int x0 = ((p0->x - camera.x) * 256) / RvR_non_zero(zoom) + RvR_xres() * 128 + 128;
         int y0 = ((p0->y - camera.y) * 256) / RvR_non_zero(zoom) + RvR_yres() * 128 + 128;
         int x1 = ((p1->x - camera.x) * 256) / RvR_non_zero(zoom) + RvR_xres() * 128 + 128;
         int y1 = ((p1->y - camera.y) * 256) / RvR_non_zero(zoom) + RvR_yres() * 128 + 128;

         int blink = j + map->sectors[i].wall_first==hover&&(RvR_frame() / 15) % 2;
         if(p0->portal!=RVR_PORT_SECTOR_INVALID)
         {
            //Only draw one wall for portals
            if(p0->portal>i)
            {
               RvR_render_line(x0, y0, x1, y1, blink?color_light_gray:color_red);
            }
         }
         else
         {
            RvR_render_line(x0, y0, x1, y1, blink?color_light_gray:color_white);
         }
      }
   }

   //Draw walls
   for(int i = 0; i<map->sector_count; i++)
   {
      for(int j = 0; j<map->sectors[i].wall_count; j++)
      {
         RvR_port_wall *p0 = map->walls + map->sectors[i].wall_first + j;

         int x0 = ((p0->x - camera.x)) / RvR_non_zero(zoom) + RvR_xres() / 2;
         int y0 = ((p0->y - camera.y)) / RvR_non_zero(zoom) + RvR_yres() / 2;

         RvR_render_rectangle(x0 - 2, y0 - 2, 5, 5, color_orange);
      }
   }

   for(int i = 0; i<map->sprite_count; i++)
   {
      RvR_port_sprite *sp = map->sprites + i;
      int x = ((sp->x - camera.x)) / RvR_non_zero(zoom) + RvR_xres() / 2;
      int y = ((sp->y - camera.y)) / RvR_non_zero(zoom) + RvR_yres() / 2;
      int rad = 128 / RvR_non_zero(zoom);
      if(x>-rad&&x<RvR_xres() + rad&&y>-rad&&y<RvR_yres() + rad)
      {
         RvR_render_circle(x, y, rad, color_aqua);
         RvR_fix22 dirx = RvR_fix22_cos(sp->dir);
         RvR_fix22 diry = RvR_fix22_sin(sp->dir);
         RvR_render_line(x * 256, y * 256, x * 256 + (dirx * (rad / 2)), y * 256 + (diry * (rad / 2)), color_aqua);

         if(sp->flags & RVR_PORT_SPRITE_WALL)
         {
            int width = RvR_texture_get(sp->tex)->width;
            RvR_fix22 p0x = x * 256 + (diry * 2 * width) / RvR_non_zero(zoom);
            RvR_fix22 p0y = y * 256 + (-dirx * 2 * width) / RvR_non_zero(zoom);
            RvR_fix22 p1x = x * 256 + (-diry * 2 * width) / RvR_non_zero(zoom);
            RvR_fix22 p1y = y * 256 + (dirx * 2 * width) / RvR_non_zero(zoom);
            RvR_render_line(p0x, p0y, p1x, p1y, color_aqua);
         }
         else if(sp->flags & RVR_PORT_SPRITE_FLOOR)
         {
            int width = RvR_texture_get(sp->tex)->width;
            int height = RvR_texture_get(sp->tex)->height;
            int half_width = (RvR_texture_get(sp->tex)->width * rad);
            RvR_fix22 p0x = x * 256 + (diry * 2 * width + dirx * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p0y = y * 256 + (-dirx * 2 * width + diry * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p1x = x * 256 + (diry * 2 * width - dirx * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p1y = y * 256 + (-dirx * 2 * width - diry * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p2x = x * 256 + (-diry * 2 * width - dirx * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p2y = y * 256 + (dirx * 2 * width - diry * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p3x = x * 256 + (-diry * 2 * width + dirx * 2 * height) / RvR_non_zero(zoom);
            RvR_fix22 p3y = y * 256 + (dirx * 2 * width + diry * 2 * height) / RvR_non_zero(zoom);
            RvR_render_line(p0x, p0y, p1x, p1y, color_aqua);
            RvR_render_line(p1x, p1y, p2x, p2y, color_aqua);
            RvR_render_line(p2x, p2y, p3x, p3y, color_aqua);
            RvR_render_line(p3x, p3y, p0x, p0y, color_aqua);
         }
      }
   }

   //Draw camera
   RvR_fix22 dirx = RvR_fix22_cos(camera.dir);
   RvR_fix22 diry = RvR_fix22_sin(camera.dir);
   int dsx = (dirx * 256) / RvR_non_zero(zoom);
   int dsy = (diry * 256) / RvR_non_zero(zoom);
   RvR_render_line(RvR_xres() * 128 + dsx / 2, RvR_yres() * 128 + dsy / 2, RvR_xres() * 128 - dsx / 2, RvR_yres() * 128 - dsy / 2, color_white);
   dirx = RvR_fix22_cos(camera.dir + 32 * 8);
   diry = RvR_fix22_sin(camera.dir + 32 * 8);
   RvR_render_line(RvR_xres() * 128 + dsx / 2, RvR_yres() * 128 + dsy / 2, RvR_xres() * 128 + dsx / 2 - (dirx * 128) / RvR_non_zero(zoom), RvR_yres() * 128 + dsy / 2 - (diry * 128) / RvR_non_zero(zoom), color_white);
   dirx = RvR_fix22_cos(camera.dir - 32 * 8);
   diry = RvR_fix22_sin(camera.dir - 32 * 8);
   RvR_render_line(RvR_xres() * 128 + dsx / 2, RvR_yres() * 128 + dsy / 2, RvR_xres() * 128 + dsx / 2 - (dirx * 128) / RvR_non_zero(zoom), RvR_yres() * 128 + dsy / 2 - (diry * 128) / RvR_non_zero(zoom), color_white);

   //Draw cursor
   int mx, my;
   if(!mouse_scroll)
   {
      RvR_mouse_pos(&mx, &my);
   }
   else
   {
      mx = RvR_xres() / 2;
      my = RvR_yres() / 2;
   }

   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);

   RvR_render_rectangle_fill(0, RvR_yres() - 12, RvR_xres(), 12, color_dark_gray);
}

static void e2d_update_view(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   camera_update();

   if(RvR_key_pressed(RVR_KEY_O))
   {
      printf("Check: %d\n", RvR_port_map_check(map));
      RvR_port_map_print_walls(map);
   }


   if(RvR_key_pressed(RVR_BUTTON_LEFT)||RvR_key_pressed(RVR_KEY_DEL)||
      RvR_key_pressed(RVR_KEY_COMMA)||RvR_key_pressed(RVR_KEY_PERIOD))
   {
      //Search for selected sprite
      sprite_sel = RVR_PORT_SPRITE_INVALID;
      int rad = 128 / RvR_non_zero(zoom);
      for(int i = 0; i<map->sprite_count; i++)
      {
         int sx = ((map->sprites[i].x - camera.x)) / RvR_non_zero(zoom) + RvR_xres() / 2;
         int sy = ((map->sprites[i].y - camera.y)) / RvR_non_zero(zoom) + RvR_yres() / 2;
         if(RvR_abs(sx - mx)<=rad&&RvR_abs(sy - my)<=rad)
            sprite_sel = (uint16_t)i;
      }
   }

   if(RvR_key_pressed(RVR_KEY_COMMA)&&sprite_sel!=RVR_PORT_SPRITE_INVALID)
   {
      undo_track_sprite_dir(sprite_sel);
      map->sprites[sprite_sel].dir += RvR_key_down(RVR_KEY_LSHIFT)?16:64;
   }
   if(RvR_key_pressed(RVR_KEY_PERIOD)&&sprite_sel!=RVR_PORT_SPRITE_INVALID)
   {
      undo_track_sprite_dir(sprite_sel);
      map->sprites[sprite_sel].dir -= RvR_key_down(RVR_KEY_LSHIFT)?16:64;
   }
   if(RvR_key_pressed(RVR_KEY_DEL)&&sprite_sel!=RVR_PORT_SPRITE_INVALID)
   {
      undo_track_sprite_del(sprite_sel);
      //TODO(Captain4LK): update references to sprites, once these are implemented
      map->sprites[sprite_sel] = map->sprites[map->sprite_count - 1];
      map->sprite_count--;
      map->sprites = RvR_realloc(map->sprites, sizeof(*map->sprites) * map->sprite_count, "Map sprites grow");
      sprite_sel = RVR_PORT_SPRITE_INVALID;
   }

   if(RvR_key_pressed(RVR_BUTTON_LEFT))
   {
      if(sprite_sel!=RVR_PORT_SPRITE_INVALID)
      {
         undo_track_sprite_pos(sprite_sel);
         sprite_move = sprite_sel;
         state = STATE2D_SPRITE_MOVE;
      }
      else
      {
         //Check for selected wall
         wall_move = RVR_PORT_WALL_INVALID;
         for(int i = 0; i<map->wall_count; i++)
         {
            RvR_port_wall *p0 = map->walls + i;
            int x0 = ((p0->x - camera.x)) / RvR_non_zero(zoom) + RvR_xres() / 2;
            int y0 = ((p0->y - camera.y)) / RvR_non_zero(zoom) + RvR_yres() / 2;

            if(mx>=x0 - 3&&mx<=x0 + 3&&my>=y0 - 3&&my<=y0 + 3)
            {
               wall_move = (uint16_t)i;
               wall_move_x = p0->x;
               wall_move_y = p0->y;
               state = STATE2D_WALL_MOVE;
               break;
            }
         }
      }
   }

   if(RvR_key_pressed(RVR_KEY_L))
      printf("map check: %d\n", RvR_port_map_check(map));

   if(RvR_key_pressed(RVR_KEY_G))
      draw_grid = (draw_grid + 1) & 7;

   if(RvR_key_pressed(RVR_KEY_SPACE))
   {
      RvR_fix22 x = ((mx + scroll_x) * zoom);
      RvR_fix22 y = ((my + scroll_y) * zoom);
      if(draw_grid_sizes[draw_grid]>0)
      {
         RvR_fix22 dgrid = 1 << draw_grid_sizes[draw_grid];
         x += dgrid / 2;
         y += dgrid / 2;
         x &= ~(x & (dgrid - 1));
         y &= ~(y & (dgrid - 1));
      }

      sector_draw_start(x, y);
      state = STATE2D_SECTOR;
   }

   if(RvR_key_pressed(RVR_BUTTON_RIGHT))
   {
      RvR_mouse_relative(1);

      mouse_scroll = 1;
      camera.x = ((scroll_x + mx) * zoom);
      camera.y = ((scroll_y + my) * zoom);
      state = STATE2D_VIEW_SCROLL;
   }

   scroll_x = (camera.x ) / RvR_non_zero(zoom) - RvR_xres() / 2;
   scroll_y = (camera.y ) / RvR_non_zero(zoom) - RvR_yres() / 2;

   if(RvR_key_pressed(RVR_KEY_NP_ADD)&&zoom>1)
      zoom -= 1;
   if(RvR_key_pressed(RVR_KEY_NP_SUB)&&zoom<1024)
      zoom += 1;

   //Check for wall hover
   hover = -1;
   RvR_fix22 x = ((mx + scroll_x) * zoom);
   RvR_fix22 y = ((my + scroll_y) * zoom);
   for(int i = 0; i<map->wall_count; i++)
   {
      int64_t x0 = map->walls[i].x;
      int64_t y0 = map->walls[i].y;
      int64_t x1 = map->walls[map->walls[i].p2].x;
      int64_t y1 = map->walls[map->walls[i].p2].y;

      int64_t t = -(1024 * ((x0 - x) * (x1 - x0) + (y0 - y) * (y1 - y0))) / RvR_non_zero((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
      if(t<0||t>1024)
         continue;

      int64_t dist = (x1 - x0) * (y0 - y) - (y1 - y0) * (x0 - x);
      dist = (dist * dist) / RvR_non_zero((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
      if(dist>36 * zoom * zoom)
         continue;

      hover = (uint16_t)i;

      if(RvR_key_pressed(RVR_KEY_INS))
      {
         uint16_t nwall = RvR_port_wall_insert(map, (uint16_t)i, x, y);
         undo_track_wall_insert(nwall);

         RvR_fix22 dgrid = 1 << draw_grid_sizes[draw_grid];
         int nx = x + dgrid / 2;
         int ny = y + dgrid / 2;
         nx &= ~(nx & (dgrid - 1));
         ny &= ~(ny & (dgrid - 1));
         RvR_port_wall_move(map, nwall, nx, ny);

         break;
      }

      if(RvR_key_down(RVR_KEY_LCTRL))
      {
         if(RvR_key_pressed(RVR_KEY_DEL))
         {
            undo_track_sector_delete(RvR_port_wall_sector(map,i));
            RvR_port_sector_delete(map, RvR_port_wall_sector(map, i));
            break;
         }
      }

      if(RvR_key_down(RVR_KEY_LALT))
      {
         if(RvR_key_pressed(RVR_KEY_S))
         {
            //Needs to be inner subsector and not have portal
            if(RvR_port_wall_subsector(map, RvR_port_wall_sector(map, (uint16_t)i), (uint16_t)i)==0||map->walls[i].portal!=RVR_PORT_SECTOR_INVALID)
               break;

            //Make inner
            undo_track_sector_make_inner(i);
            RvR_port_sector_make_inner(map, (uint16_t)i);

            break;
         }
         else if(RvR_key_pressed(RVR_KEY_F))
         {
            //TODO(Captain4LK): select wall based on mouse position relative to wall
            undo_track_wall_make_first(i,map->walls[i].portal_wall);
            if(map->walls[i].portal!=RVR_PORT_WALL_INVALID)
            {
               RvR_port_wall_make_first(map, (uint16_t)i);
               RvR_port_wall_make_first(map, map->walls[i].portal_wall);
            }
            else
            {
               RvR_port_wall_make_first(map, (uint16_t)i);
            }
            break;
         }
      }

      break;
   }

   if(RvR_key_pressed(RVR_KEY_J))
   {
      x = ((mx + scroll_x) * zoom);
      y = ((my + scroll_y) * zoom);
      uint16_t sector = RvR_port_sector_update(map, RVR_PORT_SECTOR_INVALID, x, y);
      if(sector!=RVR_PORT_SECTOR_INVALID)
      {
         RvR_port_slope slope;
         RvR_port_slope_from_floor(map, sector, &slope);
         printf("%d\n", RvR_port_slope_height_at(&slope, x, y));

         if(sector_join==RVR_PORT_SECTOR_INVALID)
         {
            sector_join = sector;
         }
         else
         {
            undo_track_sector_join(sector_join,sector);
            RvR_port_sector_join(map, sector_join, sector);
            sector_join = RVR_PORT_SECTOR_INVALID;
         }
      }
   }

   //TODO
   if(!RvR_key_down(RVR_KEY_LALT)&&!RvR_key_down(RVR_KEY_LCTRL)&&RvR_key_pressed(RVR_KEY_S))
   {
      x = ((mx + scroll_x) * zoom);
      y = ((my + scroll_y) * zoom);

      uint16_t sprite = map->sprite_count++;
      map->sprites = RvR_realloc(map->sprites, sizeof(*map->sprites) * map->sprite_count, "Map sprites grow");
      memset(map->sprites + sprite, 0, sizeof(*map->sprites));
      map->sprites[sprite].x = x;
      map->sprites[sprite].y = y;
      map->sprites[sprite].z = 0;
      map->sprites[sprite].x_units = 16;
      map->sprites[sprite].y_units = 16;
      map->sprites[sprite].sector = RVR_PORT_SECTOR_INVALID;
      uint16_t sector = RvR_port_sector_update(map, RVR_PORT_SECTOR_INVALID, x, y);
      if(sector!=RVR_PORT_SECTOR_INVALID)
      {
         map->sprites[sprite].z = map->sectors[sector].floor;
         map->sprites[sprite].sector = sector;
      }
   }

   if(RvR_key_pressed(RVR_KEY_ENTER)&&map->sector_count>0)
      editor_set_3d();
}

static void e2d_draw_view(void)
{
   e2d_draw_base();

   char tmp[1024];
   snprintf(tmp, 1024, "x: %d y:%d z:%d ang:%d", camera.x, camera.y, camera.z, camera.dir);
   RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white);
}

static void e2d_update_view_scroll(void)
{
   int rx, ry;
   RvR_mouse_relative_pos(&rx, &ry);

   camera.x += (rx * zoom) / 1;
   camera.y += (ry * zoom) / 1;
   camera.sector = RvR_port_sector_update(map, camera.sector, camera.x, camera.y);
   if(camera.sector>=0&&camera.sector<map->sector_count)
      camera.z = map->sectors[camera.sector].floor + 1024;

   if(RvR_key_released(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 0;
      RvR_mouse_relative(0);
      RvR_mouse_set_pos(RvR_xres() / 2, RvR_yres() / 2);
      state = STATE2D_VIEW;
   }
}

static void e2d_draw_view_scroll(void)
{
   e2d_draw_base();

   char tmp[1024];
   snprintf(tmp, 1024, "x: %d y:%d z:%d ang:%d", camera.x, camera.y, camera.z, camera.dir);
   RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white);
}

static void e2d_update_wall_move(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(wall_move==RVR_PORT_WALL_INVALID)
   {
      state = STATE2D_VIEW;
      return;
   }

   if(RvR_key_released(RVR_BUTTON_LEFT))
   {
      //TODO(Captain4LK): delete walls if adjacent ones overlap after moving
      RvR_fix22 x = map->walls[wall_move].x;
      RvR_fix22 y = map->walls[wall_move].y;
      RvR_port_wall_move(map, wall_move, wall_move_x, wall_move_y);
      undo_track_wall_move(wall_move);
      RvR_port_wall_move(map, wall_move, x,y);
      wall_move = RVR_PORT_WALL_INVALID;
      state = STATE2D_VIEW;
   }

   if(wall_move!=RVR_PORT_WALL_INVALID)
   {
      RvR_fix22 x = ((mx + scroll_x) * zoom);
      RvR_fix22 y = ((my + scroll_y) * zoom);
      if(draw_grid_sizes[draw_grid]>0)
      {
         RvR_fix22 dgrid = 1 << draw_grid_sizes[draw_grid];
         x += dgrid / 2;
         y += dgrid / 2;
         x &= ~(x & (dgrid - 1));
         y &= ~(y & (dgrid - 1));
      }

      RvR_port_wall_move(map, wall_move, x, y);
   }
   else
   {
      state = STATE2D_VIEW;
   }
}

static void e2d_draw_wall_move(void)
{
   e2d_draw_base();
}

static void e2d_update_sprite_move(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_released(RVR_BUTTON_LEFT))
   {
      sprite_move = RVR_PORT_SPRITE_INVALID;
      state = STATE2D_VIEW;
   }

   if(sprite_move!=RVR_PORT_SPRITE_INVALID)
   {
      RvR_fix22 x = ((mx + scroll_x) * zoom);
      RvR_fix22 y = ((my + scroll_y) * zoom);

      if(draw_grid_sizes[draw_grid]>0)
      {
         RvR_fix22 dgrid = 1 << draw_grid_sizes[draw_grid];
         x += dgrid / 2;
         y += dgrid / 2;
         x &= ~(x & (dgrid - 1));
         y &= ~(y & (dgrid - 1));
      }
      map->sprites[sprite_move].x = x;
      map->sprites[sprite_move].y = y;
   }
   else
   {
      state = STATE2D_VIEW;
   }
}

static void e2d_draw_sprite_move(void)
{
   e2d_draw_base();
}
static void e2d_update_sector(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   //Camera
   //-------------------------------------
   if(RvR_key_pressed(RVR_KEY_G))
      draw_grid = (draw_grid + 1) & 7;

   if(RvR_key_pressed(RVR_BUTTON_RIGHT))
   {
      RvR_mouse_relative(1);

      mouse_scroll = 1;
      camera.x = ((scroll_x + mx) * zoom);
      camera.y = ((scroll_y + my) * zoom);
   }

   if(mouse_scroll)
   {
      int rx, ry;
      RvR_mouse_relative_pos(&rx, &ry);

      camera.x += (rx * zoom) / 1;
      camera.y += (ry * zoom) / 1;
      if(camera.sector>=0&&camera.sector<map->sector_count)
         camera.z = map->sectors[camera.sector].floor + 512;
   }

   if(RvR_key_released(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 0;
      RvR_mouse_relative(0);
      RvR_mouse_set_pos(RvR_xres() / 2, RvR_yres() / 2);
   }

   scroll_x = (camera.x ) / RvR_non_zero(zoom) - RvR_xres() / 2;
   scroll_y = (camera.y ) / RvR_non_zero(zoom) - RvR_yres() / 2;

   if(RvR_key_pressed(RVR_KEY_NP_ADD)&&zoom>1)
      zoom -= 1;
   if(RvR_key_pressed(RVR_KEY_NP_SUB)&&zoom<1024)
      zoom += 1;
   //-------------------------------------

   world_mx = ((mx + scroll_x) * zoom);
   world_my = ((my + scroll_y) * zoom);
   if(draw_grid_sizes[draw_grid]>0)
   {
      RvR_fix22 dgrid = 1 << draw_grid_sizes[draw_grid];
      world_mx += dgrid / 2;
      world_my += dgrid / 2;
      world_mx &= ~(world_mx & (dgrid - 1));
      world_my &= ~(world_my & (dgrid - 1));
   }

   if(RvR_key_pressed(RVR_KEY_SPACE))
   {
      int ret = sector_draw_add(world_mx, world_my);
      if(ret)
         state = STATE2D_VIEW;
   }

   if(RvR_key_pressed(RVR_KEY_BACK))
   {
      if(sector_draw_back())
         state = STATE2D_VIEW;
   }
}

static void e2d_draw_sector(void)
{
   e2d_draw_base();

   sector_draw_draw(world_mx, world_my, zoom);
}
//-------------------------------------
