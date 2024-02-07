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
   STATE2D_SECTOR,
}State2D;
//-------------------------------------

//Variables
static int scroll_x = 0;
static int scroll_y = 0;
//static int grid_size = 24;
static int mouse_scroll = 0;
static char menu_input[512] = {0};

//static Map_list *map_list = NULL;
//static int map_list_scroll = 0;

static Map_sprite *sprite_sel = NULL;

static int16_t wall_move = -1;
static int16_t hover = -1;

static State2D state = STATE2D_VIEW;

static RvR_fix22 world_mx;
static RvR_fix22 world_my;

static RvR_fix22 zoom = 32;
static int draw_grid = 0;
static const RvR_fix22 draw_grid_sizes[8] = { 10, 9, 8, 7, 6, 5, 4, 0, };
//-------------------------------------

//Function prototypes
static void e2d_draw_base(void);

static void e2d_update_view(void);
static void e2d_draw_view(void);
static void e2d_update_view_scroll(void);
static void e2d_draw_view_scroll(void);
static void e2d_update_wall_move(void);
static void e2d_draw_wall_move(void);
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
   case STATE2D_SECTOR: e2d_update_sector(); break;
   }
#if 0
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   //Big mess
   //Probably the worst code I've written this year...
   if(menu!=0)
   {
      switch(menu)
      {
      case 9:
         if(RvR_key_pressed(RVR_KEY_BACK))
            menu = 0;
         if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            RvR_text_input_end();
            sprite_sel->extra0 = atoi(menu_input);
            menu = 0;
         }
         break;
      case 10:
         if(RvR_key_pressed(RVR_KEY_BACK))
            menu = 0;
         if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            RvR_text_input_end();
            sprite_sel->extra1 = atoi(menu_input);
            menu = 0;
         }
         break;
      case 11:
         if(RvR_key_pressed(RVR_KEY_BACK))
            menu = 0;
         if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            RvR_text_input_end();
            sprite_sel->extra2 = atoi(menu_input);
            menu = 0;
         }
         break;
      }

      return;
   }

   //TODO(Captain4LK): Sigh... swapping escape and caps (vim...) is currently
   //broken in sdl2 (both keys get reported as capslock...) so we can't
   //currently use escape until that's fixed
   if(RvR_key_pressed(RVR_KEY_BACK))
      menu = !menu;

   //Find selected sprite
   sprite_sel = map_sprites;
   while(sprite_sel!=NULL)
   {
      int sx = (sprite_sel->x * grid_size) / 1024- scroll_x;
      int sy = (sprite_sel->y * grid_size) /  1024- scroll_y;
      if(mx>sx - grid_size / 4&&mx<sx + grid_size / 4&&my>sy - grid_size / 4&&my<sy + grid_size / 4)
         break;
      sprite_sel = sprite_sel->next;
   }

   if(RvR_key_pressed(RVR_KEY_1)&&sprite_sel!=NULL)
   {
      menu = 9;
      snprintf(menu_input, 512, "%" PRIi32, sprite_sel->extra0);
      RvR_text_input_start(menu_input, 512);
   }
   else if(RvR_key_pressed(RVR_KEY_2)&&sprite_sel!=NULL)
   {
      menu = 10;
      snprintf(menu_input, 512, "%" PRIi32, sprite_sel->extra1);
      RvR_text_input_start(menu_input, 512);
   }
   else if(RvR_key_pressed(RVR_KEY_3)&&sprite_sel!=NULL)
   {
      menu = 11;
      snprintf(menu_input, 512, "%" PRIi32, sprite_sel->extra2);
      RvR_text_input_start(menu_input, 512);
   }

   static Map_sprite *sprite_move = NULL;

   if(RvR_key_pressed(RVR_BUTTON_LEFT))
   {
      //Check for selected wall
      wall_move = -1;
      for(int i = 0;i<map->wall_count;i++)
      { 
         RvR_port_wall *p0 = map->walls+i;
         int x0 = ((p0->x-camera.x)*grid_size)/1024+RvR_xres()/2;
         int y0 = ((p0->y-camera.y)*grid_size)/1024+RvR_yres()/2;

         if(mx>=x0-3&&mx<=x0+3&&my>=y0-3&&my<=y0+3)
         {
            wall_move = i;
            undo_track_wall_move(wall_move,p0->x,p0->y);
            break;
         }
      }

      if(wall_move==-1&&sprite_sel!=NULL)
         sprite_move = sprite_sel;
   }
   if(RvR_key_released(RVR_BUTTON_LEFT))
   {
      if(sprite_move!=NULL)
         sprite_move = NULL;
      if(wall_move>=0)
         wall_move = -1;
   }

   if(sprite_move!=NULL)
   {
      sprite_move->x = ((mx + scroll_x) * 1024) / grid_size;
      sprite_move->y = ((my + scroll_y) * 1024) / grid_size;
      sprite_move->z = 1024;
      //sprite_move->z = RvR_ray_map_floor_height_at(map, sprite_move->x / 65536, sprite_move->y / 65536);
   }
   else if(sprite_sel!=NULL)
   {
      if(RvR_key_pressed(RVR_KEY_DEL))
      {
         map_sprite_free(sprite_sel);
         sprite_sel = NULL;
      }
      else if(RvR_key_pressed(RVR_KEY_PERIOD))
         sprite_sel->direction -= RvR_key_down(RVR_KEY_LSHIFT)?32 * 16:8 * 16;
      else if(RvR_key_pressed(RVR_KEY_COMMA))
         sprite_sel->direction += RvR_key_down(RVR_KEY_LSHIFT)?32 * 16:8 * 16;
   }

   if(RvR_key_pressed(RVR_KEY_S))
   {
      Map_sprite *ms = map_sprite_new();

      ms->texture = 0;
      ms->direction = 0;
      ms->extra0 = 0;
      ms->extra1 = 0;
      ms->extra2 = 0;
      ms->flags = 0;
      ms->x = ((mx + scroll_x) * 1024) / grid_size;
      ms->y = ((my + scroll_y) * 1024) / grid_size;
      ms->z = 1024;
      //ms->z = RvR_ray_map_floor_height_at(map, ms->x / 65536, ms->y / 65536);

      map_sprite_add(ms);
   }

   scroll_x = (camera.x * grid_size) / 1024- RvR_xres() / 2;
   scroll_y = (camera.y * grid_size) / 1024- RvR_yres() / 2;

#endif
}

void editor2d_draw(void)
{
   switch(state)
   {
   case STATE2D_VIEW: e2d_draw_view(); break;
   case STATE2D_VIEW_SCROLL: e2d_draw_view_scroll(); break;
   case STATE2D_WALL_MOVE: e2d_draw_wall_move(); break;
   case STATE2D_SECTOR: e2d_draw_sector(); break;
   }

#if 0
   char tmp[1024];

   switch(menu)
   {
   case -2: snprintf(tmp, 1024, "Saved map to %s", map_path_get()); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case -1: RvR_render_string(5, RvR_yres() - 10, 1, "Invalid input", color_white); break;
   case 0: snprintf(tmp, 1024, "x: %d y:%d ang:%d", camera.x, camera.y, camera.dir); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case 1: RvR_render_string(5, RvR_yres() - 10, 1, "(N)ew, (L)oad, (S)ave , save (A)s, (Q)uit", color_white); break;
   case 2: RvR_render_string(5, RvR_yres() - 10, 1, "Are you sure you want to start a new map? (Y/N)", color_white); break;
   case 3: snprintf(tmp, 1024, "Map width: %s", menu_input); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case 4: snprintf(tmp, 1024, "Map height: %s", menu_input); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case 5: snprintf(tmp, 1024, "Save as: %s", menu_input); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case 6: RvR_render_string(5, RvR_yres() - 10, 1, "Are you sure you want to quit? (Y/N)", color_white); break;
   case 7: RvR_render_string(5, RvR_yres() - 10, 1, "Save changes? (Y/N)", color_white); break;
   case 9: snprintf(tmp, 1024, "Sprite (texture %" PRIu16 ") extra0: %s", sprite_sel->texture, menu_input); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case 10: snprintf(tmp, 1024, "Sprite (texture %" PRIu16 ") extra1: %s", sprite_sel->texture, menu_input); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   case 11: snprintf(tmp, 1024, "Sprite (texture %" PRIu16 ") extra2: %s", sprite_sel->texture, menu_input); RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white); break;
   }
#endif
}

static void e2d_draw_base(void)
{
   RvR_render_clear(color_black);

   /*//Draw sprites
   Map_sprite *sp = map_sprites;
   while(sp!=NULL)
   {
      int x = (sp->x * grid_size) / 65536 - scroll_x;
      int y = (sp->y * grid_size) / 65536 - scroll_y;
      if(x>-grid_size * 2&&x<RvR_xres() + grid_size * 2&&y>-grid_size * 2&&y<RvR_yres() + grid_size * 2)
      {
         RvR_render_circle(x, y, grid_size / 4, color_white);
         RvR_fix16 dirx = RvR_fix16_cos(sp->direction);
         RvR_fix16 diry = RvR_fix16_sin(sp->direction);
         //RvR_fix22_vec2 direction = RvR_fix22_vec2_rot(sp->direction);
         RvR_render_line(x * 256, y * 256, x * 256 + (dirx * (grid_size / 2)) / 256, y * 256 + (diry * (grid_size / 2)) / 256, color_white);

         if(sp->flags & 8)
         {
            int half_width = (RvR_texture_get(sp->texture)->width * grid_size * 2) / 256;
            RvR_fix16 p0x = x * 256 + (diry * half_width) / 256;
            RvR_fix16 p0y = y * 256 + (-dirx * half_width) / 256;
            RvR_fix16 p1x = x * 256 + (-diry * half_width) / 256;
            RvR_fix16 p1y = y * 256 + (dirx * half_width) / 256;
            RvR_render_line(p0x, p0y, p1x, p1y, color_white);
         }
      }

      sp = sp->next;
   }*/

   //Draw grid
   //printf("%d\n",(RvR_xres()*1024)/(grid_size*draw_grid_size));
   if(draw_grid_sizes[draw_grid]>0&&(RvR_yres()*zoom)/((1<<draw_grid_sizes[draw_grid]))<=RvR_yres()/2)
   {
      int dgrid = 1<<draw_grid_sizes[draw_grid];
      int start = -((RvR_yres()/2)*zoom)/RvR_non_zero(dgrid);
      int end = +((RvR_yres()/2)*zoom)/RvR_non_zero(dgrid);
      for(int y = start;y<=end;y++)
      {
         RvR_fix22 wy = -(camera.y%dgrid)+y*dgrid+(camera.y);
         RvR_render_horizontal_line(0,RvR_xres(),(wy-camera.y)/RvR_non_zero(zoom)+RvR_yres()/2,color_dark_gray);
      }

      start = -((RvR_xres()/2)*zoom)/RvR_non_zero(dgrid);
      end = +((RvR_xres()/2)*zoom)/RvR_non_zero(dgrid);
      for(int x = start;x<=end;x++)
      {
         RvR_fix22 wx = -(camera.x%dgrid)+x*dgrid+(camera.x);
         RvR_render_vertical_line((wx-camera.x)/RvR_non_zero(zoom)+RvR_xres()/2,0,RvR_yres(),color_dark_gray);
      }
   }

   //Draw sectors
   for(int i = 0;i<map->sector_count;i++)
   {
      //if(i==2)
         //continue;
      for(int j = 0;j<map->sectors[i].wall_count;j++)
      {
         RvR_port_wall *p0 = map->walls+map->sectors[i].wall_first+j;
         RvR_port_wall *p1 = map->walls+p0->p2;

         if(p0->p2==-1)
            continue;

         int x0 = ((p0->x-camera.x)*256)/RvR_non_zero(zoom)+RvR_xres()*128+128;
         int y0 = ((p0->y-camera.y)*256)/RvR_non_zero(zoom)+RvR_yres()*128+128;
         int x1 = ((p1->x-camera.x)*256)/RvR_non_zero(zoom)+RvR_xres()*128+128;
         int y1 = ((p1->y-camera.y)*256)/RvR_non_zero(zoom)+RvR_yres()*128+128;

         int blink = j+map->sectors[i].wall_first==hover&&(RvR_frame()/15)%2;
         if(p0->portal>=0)
         {
            //Only draw one wall for portals
            if(p0->portal>i)
            {
               RvR_render_line(x0,y0,x1,y1,blink?color_light_gray:color_red);
            }
         }
         else
         {
            RvR_render_line(x0,y0,x1,y1,blink?color_light_gray:color_white);
         }
      }
   }

   //Draw walls
   for(int i = 0;i<map->sector_count;i++)
   {
      for(int j = 0;j<map->sectors[i].wall_count;j++)
      {
         RvR_port_wall *p0 = map->walls+map->sectors[i].wall_first+j;

         int x0 = ((p0->x-camera.x))/RvR_non_zero(zoom)+RvR_xres()/2;
         int y0 = ((p0->y-camera.y))/RvR_non_zero(zoom)+RvR_yres()/2;

         RvR_render_rectangle(x0-2,y0-2,5,5,color_orange);
      }
   }

   //Draw camera
   //RvR_fix22_vec2 direction = RvR_fix22_vec2_rot(camera.direction);
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

   //RvR_fix22 x = ((mx+scroll_x)*zoom);
   //RvR_fix22 y = ((my+scroll_y)*zoom);
   //printf("%d\n",RvR_port_sector_update(map,-1,x,y));

   camera_update();

   if(RvR_key_pressed(RVR_KEY_O))
   {
      printf("Check: %d\n",RvR_port_map_check(map));
      RvR_port_map_print_walls(map);
   }

   if(RvR_key_pressed(RVR_BUTTON_LEFT))
   {
      //Check for selected wall
      wall_move = -1;
      for(int i = 0;i<map->wall_count;i++)
      { 
         RvR_port_wall *p0 = map->walls+i;
         int x0 = ((p0->x-camera.x))/RvR_non_zero(zoom)+RvR_xres()/2;
         int y0 = ((p0->y-camera.y))/RvR_non_zero(zoom)+RvR_yres()/2;

         if(mx>=x0-3&&mx<=x0+3&&my>=y0-3&&my<=y0+3)
         {
            wall_move = i;
            undo_track_wall_move(wall_move,p0->x,p0->y);
            state = STATE2D_WALL_MOVE;
            break;
         }
      }

      //if(wall_move==-1&&sprite_sel!=NULL)
         //sprite_move = sprite_sel;
   }

   if(RvR_key_pressed(RVR_KEY_L))
      printf("map check: %d\n",RvR_port_map_check(map));

   if(RvR_key_pressed(RVR_KEY_G))
      draw_grid = (draw_grid+1)&7;

   if(RvR_key_pressed(RVR_KEY_SPACE))
   {
      RvR_fix22 x = ((mx+scroll_x)*zoom);
      RvR_fix22 y = ((my+scroll_y)*zoom);
      if(draw_grid_sizes[draw_grid]>0)
      {
         RvR_fix22 dgrid = 1<<draw_grid_sizes[draw_grid];
         x+=dgrid/2;
         y+=dgrid/2;
         x&=~(x&(dgrid-1));
         y&=~(y&(dgrid-1));
      }

      sector_draw_start(x,y);
      state = STATE2D_SECTOR;
      /*sector_current = RvR_port_sector_update(map,-1,x,y);

      if(sector_current==-1)
      {
         sector_current = RvR_port_sector_new(map,x,y);
         map->sectors[sector_current].floor = 0;
         map->sectors[sector_current].ceiling = 2*1024;
         map->sectors[sector_current].floor_tex = 15;
         map->sectors[sector_current].ceiling_tex = 15;

         state = STATE2D_SECTOR;
      }
      else
      {
         RvR_port_wall_append(map,sector_current,x,y);
         state = STATE2D_SECTOR;
      }*/
   }

   if(RvR_key_pressed(RVR_BUTTON_RIGHT))
   {
      RvR_mouse_relative(1);

      mouse_scroll = 1;
      camera.x = ((scroll_x + mx) * zoom);
      camera.y = ((scroll_y + my) * zoom);
      state = STATE2D_VIEW_SCROLL;
   }

   scroll_x = (camera.x ) / RvR_non_zero(zoom)- RvR_xres() / 2;
   scroll_y = (camera.y ) / RvR_non_zero(zoom)- RvR_yres() / 2;

   if(RvR_key_pressed(RVR_KEY_NP_ADD)&&zoom>1)
      zoom-=1;
   if(RvR_key_pressed(RVR_KEY_NP_SUB)&&zoom<1024)
      zoom+=1;

   //Check for wall hover
   hover = -1;
   RvR_fix22 x = ((mx+scroll_x)*zoom);
   RvR_fix22 y = ((my+scroll_y)*zoom);
   for(int i = 0;i<map->wall_count;i++)
   {
      int64_t x0 = map->walls[i].x;
      int64_t y0 = map->walls[i].y;
      int64_t x1 = map->walls[map->walls[i].p2].x;
      int64_t y1 = map->walls[map->walls[i].p2].y;

      int64_t t = -(1024*((x0-x)*(x1-x0)+(y0-y)*(y1-y0)))/RvR_non_zero((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
      if(t<0||t>1024)
         continue;
      
      int64_t dist = (x1-x0)*(y0-y)-(y1-y0)*(x0-x);
      dist = (dist*dist)/RvR_non_zero((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
      if(dist>36*zoom*zoom)
         continue;

      hover = i;

      if(RvR_key_pressed(RVR_KEY_INS))
      {
         int16_t nwall = RvR_port_wall_insert(map,i,x,y);

         RvR_fix22 dgrid = 1<<draw_grid_sizes[draw_grid];
         int nx = x+dgrid/2;
         int ny = y+dgrid/2;
         nx&=~(nx&(dgrid-1));
         ny&=~(ny&(dgrid-1));
         RvR_port_wall_move(map,nwall,nx,ny);

         break;
      }

      if(RvR_key_pressed(RVR_KEY_S)&&RvR_key_down(RVR_KEY_LALT))
      {
         //Needs to be inner subsector and not have portal
         if(RvR_port_wall_subsector(map,RvR_port_wall_sector(map,i),i)==0||map->walls[i].portal>=0)
            break;

         //Make inner
         RvR_port_sector_make_inner(map,i);

         break;
      }

      break;
      //printf("%d %ld\n",i,dist<36*zoom*zoom);
      //int64_t d0 = (int64_t)(x1-x)*(x1-x)+(int64_t)(y1-y)*(y1-y);
      //int64_t d1 = (int64_t)(x0-x)*(x0-x)+(int64_t)(y0-y)*(y0-y);
      //printf("%d %ld %ld\n",i,d0,d1);
   }

   /*if(RvR_key_pressed(RVR_KEY_NP_ADD)&&grid_size<64)
   {
      int scrollx = ((scroll_x + RvR_xres() / 2) * 1024) / grid_size;
      int scrolly = ((scroll_y + RvR_yres() / 2) * 1024) / grid_size;

      grid_size += 4;
      scroll_x = (scrollx * grid_size) / 1024- RvR_xres() / 2;
      scroll_y = (scrolly * grid_size) / 1024- RvR_yres() / 2;
   }
   if(RvR_key_pressed(RVR_KEY_NP_SUB)&&grid_size>4)
   {
      int scrollx = ((scroll_x + RvR_xres() / 2) * 1024) / grid_size;
      int scrolly = ((scroll_y + RvR_yres() / 2) * 1024) / grid_size;

      grid_size -= 4;
      scroll_x = (scrollx * grid_size) / 1024- RvR_xres() / 2;
      scroll_y = (scrolly * grid_size) / 1024- RvR_yres() / 2;
   }*/

   if(RvR_key_pressed(RVR_KEY_ENTER)&&map->sector_count>0)
      editor_set_3d();
}

static void e2d_draw_view(void)
{
   e2d_draw_base();

   char tmp[1024];
   snprintf(tmp, 1024, "x: %d y:%d ang:%d", camera.x, camera.y, camera.dir);
   RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white);
}

static void e2d_update_view_scroll(void)
{
   int rx, ry;
   RvR_mouse_relative_pos(&rx, &ry);

   camera.x += (rx * zoom) / 1;
   camera.y += (ry * zoom) / 1;
   camera.sector = RvR_port_sector_update(map,camera.sector,camera.x,camera.y);
   if(camera.sector>=0&&camera.sector<map->sector_count)
      camera.z = map->sectors[camera.sector].floor+1024;

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
   snprintf(tmp, 1024, "x: %d y:%d ang:%d", camera.x, camera.y, camera.dir);
   RvR_render_string(5, RvR_yres() - 10, 1, tmp, color_white);
}

static void e2d_update_wall_move(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(RvR_key_released(RVR_BUTTON_LEFT))
   {
      wall_move = -1;
      state = STATE2D_VIEW;
   }

   if(wall_move>=0)
   {
      RvR_fix22 x = ((mx+scroll_x)*zoom);
      RvR_fix22 y = ((my+scroll_y)*zoom);
      if(draw_grid_sizes[draw_grid]>0)
      {
         RvR_fix22 dgrid = 1<<draw_grid_sizes[draw_grid];
         x+=dgrid/2;
         y+=dgrid/2;
         x&=~(x&(dgrid-1));
         y&=~(y&(dgrid-1));
      }

      RvR_port_wall_move(map,wall_move,x,y);
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

static void e2d_update_sector(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   //Camera
   //-------------------------------------
   if(RvR_key_pressed(RVR_KEY_G))
      draw_grid = (draw_grid+1)&7;

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
         camera.z = map->sectors[camera.sector].floor+512;
   }

   if(RvR_key_released(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 0;
      RvR_mouse_relative(0);
      RvR_mouse_set_pos(RvR_xres() / 2, RvR_yres() / 2);
   }

   scroll_x = (camera.x ) / RvR_non_zero(zoom)- RvR_xres() / 2;
   scroll_y = (camera.y ) / RvR_non_zero(zoom)- RvR_yres() / 2;

   if(RvR_key_pressed(RVR_KEY_NP_ADD)&&zoom>1)
      zoom-=1;
   if(RvR_key_pressed(RVR_KEY_NP_SUB)&&zoom<1024)
      zoom+=1;
   //-------------------------------------

   world_mx = ((mx+scroll_x)*zoom);
   world_my = ((my+scroll_y)*zoom);
   if(draw_grid_sizes[draw_grid]>0)
   {
      RvR_fix22 dgrid = 1<<draw_grid_sizes[draw_grid];
      world_mx+=dgrid/2;
      world_my+=dgrid/2;
      world_mx&=~(world_mx&(dgrid-1));
      world_my&=~(world_my&(dgrid-1));
   }

   if(RvR_key_pressed(RVR_KEY_SPACE))
   {
      int ret = sector_draw_add(world_mx,world_my);
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

   sector_draw_draw(world_mx,world_my,zoom);
}
//-------------------------------------
