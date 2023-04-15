/*
RvnicRaven retro game engine

Written in 2021,2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "color.h"
#include "map.h"
#include "editor.h"
#include "editor2d.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int scroll_x = 0;
static int scroll_y = 0;
static int grid_size = 24;
static int mouse_scroll = 0;
static int menu = 0;
static char menu_input[512] = {0};
static uint16_t menu_new_width = 0;
static uint16_t menu_new_height = 0;

static Map_list *map_list = NULL;
static int map_list_scroll = 0;

static Map_sprite *sprite_sel = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor2d_update()
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   //Big mess
   //Probably the worst code I've written this year...
   if(menu!=0)
   {
      switch(menu)
      {
      case -2:
         if(RvR_key_pressed(RVR_KEY_BACK))
            menu = 0;
         break;
      case -1:
         if(RvR_key_pressed(RVR_KEY_BACK))
            menu = 0;
         break;
      case 1:
         if(RvR_key_pressed(RVR_KEY_BACK))
            menu = 0;

         if(RvR_key_pressed(RVR_KEY_N))
         {
            menu = 2;
         }
         else if(RvR_key_pressed(RVR_KEY_S))
         {
            map_save();
            menu = -2;
         }
         else if(RvR_key_pressed(RVR_KEY_A))
         {
            menu = 5;
            menu_input[0] = '\0';
            RvR_text_input_start(menu_input, 64);
         }
         else if(RvR_key_pressed(RVR_KEY_Q))
         {
            menu = 6;
         }
         else if(RvR_key_pressed(RVR_KEY_L))
         {
            map_list = map_list_get();
            menu = 8;
         }
         break;
      case 2:
         if(RvR_key_pressed(RVR_KEY_BACK)||RvR_key_pressed(RVR_KEY_N))
         {
            menu = 0;
         }

         if(RvR_key_pressed(RVR_KEY_Y))
         {
            menu_input[0] = '\0';
            RvR_text_input_start(menu_input, 512);
            menu = 3;
         }
         break;
      case 3:
         if(RvR_key_pressed(RVR_KEY_BACK))
         {
            RvR_text_input_end();
            menu = 0;
         }

         if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            RvR_text_input_end();
            menu_new_width = atoi(menu_input);
            if(menu_new_width<=0)
            {
               menu = -1;
            }
            else
            {
               menu_input[0] = '\0';
               RvR_text_input_start(menu_input, 512);
               menu = 4;
            }
         }
         break;
      case 4:
         if(RvR_key_pressed(RVR_KEY_BACK))
         {
            RvR_text_input_end();
            menu = 0;
         }

         if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            RvR_text_input_end();
            menu_new_height = atoi(menu_input);
            if(menu_new_height<=0)
            {
               menu = -1;
            }
            else
            {
               map_new(menu_new_width, menu_new_height);
               menu = 0;
            }
         }
         break;
      case 5:
         if(RvR_key_pressed(RVR_KEY_BACK))
         {
            RvR_text_input_end();
            menu = 0;
         }

         if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            RvR_text_input_end();
            map_set_path(menu_input);
            map_save();
            menu = 0;
         }
         break;
      case 6:
         if(RvR_key_pressed(RVR_KEY_Y))
            menu = 7;
         else if(RvR_key_pressed(RVR_KEY_N))
            menu = 0;
         break;
      case 7:
         if(RvR_key_pressed(RVR_KEY_Y))
         {
            map_save();
            RvR_quit();
         }
         else if(RvR_key_pressed(RVR_KEY_N))
         {
            RvR_quit();
         }
         break;
      case 8:
         if(RvR_key_pressed(RVR_KEY_BACK))
         {
            menu = 0;
         }
         else if(RvR_key_pressed(RVR_KEY_DOWN)&&map_list_scroll<map_list->data_used - 1)
         {
            map_list_scroll++;
         }
         else if(RvR_key_pressed(RVR_KEY_UP)&&map_list_scroll>0)
         {
            map_list_scroll--;
         }
         else if(RvR_key_pressed(RVR_KEY_ENTER))
         {
            map_load(map_list->data[map_list_scroll]);
            menu = 0;
         }
         break;
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
      int sx = (sprite_sel->x * grid_size) / 65536 - scroll_x;
      int sy = (sprite_sel->y * grid_size) /  65536 - scroll_y;
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
   if(RvR_key_pressed(RVR_BUTTON_LEFT)&&sprite_sel!=NULL)
      sprite_move = sprite_sel;
   if(RvR_key_released(RVR_BUTTON_LEFT))
      sprite_move = NULL;

   if(sprite_move!=NULL)
   {
      sprite_move->x = ((mx + scroll_x) * 65536) / grid_size;
      sprite_move->y = ((my + scroll_y) * 65536) / grid_size;
      sprite_move->z = RvR_ray_map_floor_height_at(map,sprite_move->x / 65536, sprite_move->y / 65536);
   }
   else if(sprite_sel!=NULL)
   {
      if(RvR_key_pressed(RVR_KEY_DEL))
      {
         map_sprite_free(sprite_sel);
         sprite_sel = NULL;
      }
      else if(RvR_key_pressed(RVR_KEY_PERIOD))
         sprite_sel->direction -= RvR_key_down(RVR_KEY_LSHIFT)?32*16:8*16;
      else if(RvR_key_pressed(RVR_KEY_COMMA))
         sprite_sel->direction += RvR_key_down(RVR_KEY_LSHIFT)?32*16:8*16;
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
      ms->x = ((mx + scroll_x) * 65536) / grid_size;
      ms->y = ((my + scroll_y) * 65536) / grid_size;
      ms->z = RvR_ray_map_floor_height_at(map,ms->x / 65536, ms->y / 65536);

      map_sprite_add(ms);
   }

   if(RvR_key_pressed(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 1;
      RvR_mouse_relative(1);

      camera.x = ((scroll_x + mx) * 65536) / grid_size;
      camera.y = ((scroll_y + my) * 65536) / grid_size;
   }

   if(RvR_key_released(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 0;
      RvR_mouse_relative(0);
      RvR_mouse_set_pos(RvR_xres()/ 2, RvR_yres()/ 2);
   }

   if(mouse_scroll)
   {
      int rx, ry;
      RvR_mouse_relative_pos(&rx, &ry);

      camera.x += (rx * 65536) / grid_size;
      camera.y += (ry * 65536) / grid_size;
   }
   else
   {
      camera_update();
   }

   scroll_x = (camera.x * grid_size) / 65536- RvR_xres()/ 2;
   scroll_y = (camera.y * grid_size) / 65536- RvR_yres()/ 2;

   if(RvR_key_pressed(RVR_KEY_NP_ADD)&&grid_size<64)
   {
      int scrollx = ((scroll_x + RvR_xres()/ 2) * 65536) / grid_size;
      int scrolly = ((scroll_y + RvR_yres()/ 2) * 65536) / grid_size;

      grid_size += 4;
      scroll_x = (scrollx * grid_size) / 65536- RvR_xres()/ 2;
      scroll_y = (scrolly * grid_size) / 65536- RvR_yres()/ 2;
   }
   if(RvR_key_pressed(RVR_KEY_NP_SUB)&&grid_size>4)
   {
      int scrollx = ((scroll_x + RvR_xres()/ 2) * 65536) / grid_size;
      int scrolly = ((scroll_y + RvR_yres()/ 2) * 1024) / grid_size;

      grid_size -= 4;
      scroll_x = (scrollx * grid_size) / 65536- RvR_xres()/ 2;
      scroll_y = (scrolly * grid_size) / 65536- RvR_yres()/ 2;
   }
}

void editor2d_draw()
{
   RvR_render_clear(color_black);

   if(menu==8)
   {
      int scroll = 0;
      if(map_list_scroll>RvR_yres()/ 10)
         scroll = map_list_scroll - RvR_yres()- 10;
      for(int i = 0; i<=RvR_yres()/ 10; i++)
      {
         int index = i + scroll;
         if(index<map_list->data_used)
            RvR_render_string(5, i * 10, 1, map_list->data[i], index==map_list_scroll?color_white:color_light_gray);
      }
      return;
   }

   int sx = scroll_x / grid_size;
   int sy = scroll_y / grid_size;
   for(int y = -1; y<=RvR_yres()/ grid_size + 1; y++)
   {
      for(int x = -1; x<=RvR_xres()/ grid_size + 1; x++)
      {
         {
            int tx = (x + sx) * grid_size - scroll_x;
            int ty = (y + sy) * grid_size - scroll_y;

            uint16_t ftex = RvR_ray_map_floor_tex_at(map,x + sx, y + sy);
            uint16_t ctex = RvR_ray_map_ceil_tex_at(map,x + sx, y + sy);
            RvR_fix16 fheight = RvR_ray_map_floor_height_at(map,x + sx, y + sy);
            RvR_fix16 cheight = RvR_ray_map_ceiling_height_at(map,x + sx, y + sy);

            if(!map_tile_comp(ftex, ctex, fheight, cheight, x + sx, y + sy - 1))
               RvR_render_horizontal_line(tx, tx + grid_size, ty, color_light_gray);
            if(!map_tile_comp(ftex, ctex, fheight, cheight, x + sx - 1, y + sy))
               RvR_render_vertical_line(tx, ty, ty + grid_size, color_light_gray);
         }
      }
   }

   //Draw sprites
   Map_sprite *sp = map_sprites;
   while(sp!=NULL)
   {
      int x = (sp->x * grid_size) / 65536- scroll_x;
      int y = (sp->y * grid_size) / 65536- scroll_y;
      if(x>-grid_size * 2&&x<RvR_xres()+ grid_size * 2&&y>-grid_size * 2&&y<RvR_yres()+ grid_size * 2)
      {
         RvR_render_circle(x, y, grid_size / 4, color_white);
         RvR_fix16 dirx = RvR_fix16_cos(sp->direction);
         RvR_fix16 diry = RvR_fix16_sin(sp->direction);
         //RvR_fix22_vec2 direction = RvR_fix22_vec2_rot(sp->direction);
         RvR_render_line(x*256, y*256, x*256+ (dirx * (grid_size / 2))/256 , y *256+ (diry * (grid_size / 2))/256, color_white);

         if(sp->flags & 8)
         {
            int half_width = (RvR_texture_get(sp->texture)->width * 8 * grid_size) / 65536;
            RvR_fix16 p0x = (diry * half_width) + x*65536;
            RvR_fix16 p0y = (-dirx * half_width) + y*65536;
            RvR_fix16 p1x = (-diry * half_width) + x*65536;
            RvR_fix16 p1y = (dirx * half_width) + y*65536;
            RvR_render_line(p0x, p0y, p1x, p1y, color_white);
         }
      }

      sp = sp->next;
   }

   //Draw camera
   //RvR_fix22_vec2 direction = RvR_fix22_vec2_rot(camera.direction);
   RvR_fix16 dirx = RvR_fix16_cos(camera.dir);
   RvR_fix16 diry = RvR_fix16_sin(camera.dir);
   int dsx = (dirx * grid_size)/256 ;
   int dsy = (diry * grid_size)/256 ;
   RvR_render_line(RvR_xres() *128+ dsx / 2, RvR_yres()*128+ dsy / 2, RvR_xres()*128- dsx / 2, RvR_yres()*128- dsy / 2, color_white);
   dirx = RvR_fix16_cos(camera.dir+32*128);
   diry = RvR_fix16_sin(camera.dir+32*128);
   RvR_render_line(RvR_xres()*128 + dsx / 2, RvR_yres()*128 + dsy / 2, RvR_xres()*128 + dsx / 2 - (dirx * grid_size / 2) / 256, RvR_yres()*128 + dsy / 2 - (diry * grid_size / 2) / 256, color_white);
   dirx = RvR_fix16_cos(camera.dir-32*128);
   diry = RvR_fix16_sin(camera.dir-32*128);
   RvR_render_line(RvR_xres()*128 + dsx / 2, RvR_yres()*128 + dsy / 2, RvR_xres()*128 + dsx / 2 - (dirx * grid_size / 2) / 256, RvR_yres()*128 + dsy / 2 - (diry * grid_size / 2) / 256, color_white);

   //Draw cursor
   int mx, my;
   if(!mouse_scroll)
   {
      RvR_mouse_pos(&mx, &my);
   }
   else
   {
      mx = RvR_xres()/ 2;
      my = RvR_yres()/ 2;
   }

   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);

   RvR_render_rectangle_fill(0, RvR_yres()- 12, RvR_xres(), 12, color_dark_gray);
   char tmp[1024];

   switch(menu)
   {
   case -2: snprintf(tmp, 1024, "Saved map to %s", map_path_get()); RvR_render_string(5, RvR_yres()- 10, 1, tmp, color_white); break;
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
}
//-------------------------------------
