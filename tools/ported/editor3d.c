/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int16_t wx = 0;
static int16_t wy = 0;
static int menu = 0;
static int wlocation = 0;

static uint16_t texture_selected = 0;
static int texture_selection_scroll = 0;
static int brush = 0;
static char menu_input[512] = {0};
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor3d_update(void)
{
}

void editor3d_draw(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(menu==0)
   {
      //Highlight selected tile
      static uint16_t texture_highlight_old = 65535;
      uint16_t texture_highlight = 0;

      //if(wlocation==0)
         //texture_highlight = RvR_ray_map_floor_tex_at(map, wx, wy);
      //if(wlocation==1)
         //texture_highlight = RvR_ray_map_ceil_tex_at(map, wx, wy);
      //if(wlocation==2)
         //texture_highlight = RvR_ray_map_wall_ftex_at(map, wx, wy);
      //if(wlocation==3)
         //texture_highlight = RvR_ray_map_wall_ctex_at(map, wx, wy);

      RvR_texture *texture_high = RvR_texture_get(texture_highlight);
      if(texture_high->width==64&&texture_high->height==64)
      {
         if(texture_highlight!=texture_highlight_old)
         {
            RvR_texture *texture_new = RvR_texture_get(UINT16_MAX - 1);
            memcpy(texture_new->data, texture_high->data, sizeof(*texture_new->data) * texture_high->width * texture_high->height);

            //Add outline
            for(int i = 0; i<texture_new->height; i++)
            {
               texture_new->data[i] = color_white;
               texture_new->data[i + texture_new->height] = color_white;
               texture_new->data[i + texture_new->height * (texture_new->width - 1)] = color_white;
               texture_new->data[i + texture_new->height * (texture_new->width - 2)] = color_white;
            }
            for(int i = 0; i<texture_new->width; i++)
            {
               texture_new->data[i * texture_new->height] = color_white;
               texture_new->data[i * texture_new->height + 1] = color_white;
               texture_new->data[i * texture_new->height + texture_new->height - 1] = color_white;
               texture_new->data[i * texture_new->height + texture_new->height - 2] = color_white;
            }
         }

         //if(wlocation==0)
            //RvR_ray_map_floor_tex_set(map, wx, wy, UINT16_MAX - 1);
         //if(wlocation==1)
            //RvR_ray_map_ceil_tex_set(map, wx, wy, UINT16_MAX - 1);
         //if(wlocation==2)
            //RvR_ray_map_wall_ftex_set(map, wx, wy, UINT16_MAX - 1);
         //if(wlocation==3)
            //RvR_ray_map_wall_ctex_set(map, wx, wy, UINT16_MAX - 1);
      }
      else if(texture_high->width==64&&texture_high->height==1 << RvR_log2(texture_high->height))
      {
         if(texture_highlight!=texture_highlight_old)
         {
            RvR_texture *texture_new = RvR_texture_get(UINT16_MAX - 2);
            memcpy(texture_new->data, texture_high->data, sizeof(*texture_new->data) * texture_high->width * texture_high->height);

            //Add outline
            for(int i = 0; i<texture_new->height; i++)
            {
               texture_new->data[i] = color_white;
               texture_new->data[i + texture_new->height] = color_white;
               texture_new->data[i + texture_new->height * (texture_new->width - 1)] = color_white;
               texture_new->data[i + texture_new->height * (texture_new->width - 2)] = color_white;
            }
            for(int i = 0; i<texture_new->width; i++)
            {
               texture_new->data[i * texture_new->height] = color_white;
               texture_new->data[i * texture_new->height + 1] = color_white;
               texture_new->data[i * texture_new->height + texture_new->height - 1] = color_white;
               texture_new->data[i * texture_new->height + texture_new->height - 2] = color_white;
            }
         }

         //if(wlocation==0)
            //RvR_ray_map_floor_tex_set(map, wx, wy, UINT16_MAX - 2);
         //if(wlocation==1)
            //RvR_ray_map_ceil_tex_set(map, wx, wy, UINT16_MAX - 2);
         //if(wlocation==2)
            //RvR_ray_map_wall_ftex_set(map, wx, wy, UINT16_MAX - 2);
         //if(wlocation==3)
            //RvR_ray_map_wall_ctex_set(map, wx, wy, UINT16_MAX - 2);
      }
      //-------------------------------------

      RvR_port_draw_begin(map,&camera);
      /*Map_sprite *s = map_sprites;
      while(s!=NULL)
      {
         //TODO
         RvR_ray_draw_sprite(&camera, s->x, s->y, s->z, s->direction, s->texture, s->flags);
         s = s->next;
      }*/

      RvR_port_draw_map(NULL);
      RvR_port_draw_end(NULL);

      texture_highlight_old = texture_highlight;
      //if(wlocation==0)
         //RvR_ray_map_floor_tex_set(map, wx, wy, texture_highlight);
      //if(wlocation==1)
         //RvR_ray_map_ceil_tex_set(map, wx, wy, texture_highlight);
      //if(wlocation==2)
         //RvR_ray_map_wall_ftex_set(map, wx, wy, texture_highlight);
      //if(wlocation==3)
         //RvR_ray_map_wall_ctex_set(map, wx, wy, texture_highlight);

      RvR_render_rectangle(8, RvR_yres() - 74, 66, 66, color_white);
      draw_fit64(9, RvR_yres() - 73, texture_selected);
   }
   else if(menu==1||menu==3)
   {
      RvR_render_clear(color_black);

      for(int y = 0; y<=RvR_yres() / 64; y++)
      {
         for(int x = 0; x<=RvR_xres() / 64; x++)
         {
            int index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
            index = texture_list_used_wrap(texture_list_used.data_last - index);
            draw_fit64(x * 64, y * 64, texture_list_used.data[index]);

            RvR_render_font_set(0xF001);
            const char tmp_font[16];
            snprintf(tmp_font, 16, "%d", texture_list_used.data[index]);
            RvR_render_rectangle_fill(x * 64, y * 64, strlen(tmp_font) * 4 + 1, 7, color_black);
            RvR_render_string(x * 64 + 1, y * 64 + 1, 1, tmp_font, color_yellow);
            RvR_render_font_set(0xF000);
         }
      }

      if(mx / 64<RvR_xres() / 64)
         RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);
   }
   else if(menu==2||menu==4)
   {
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
               const char tmp_font[16];
               snprintf(tmp_font, 16, "%d", texture_list.data[index]);
               RvR_render_rectangle_fill(x * 64, y * 64, strlen(tmp_font) * 4 + 1, 7, color_black);
               RvR_render_string(x * 64 + 1, y * 64 + 1, 1, tmp_font, color_yellow);
               RvR_render_font_set(0xF000);
            }
         }
      }

      if(mx / 64<RvR_xres() / 64)
         RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);
   }

   if(menu==3||menu==4)
   {
      RvR_render_rectangle_fill(0, 0, RvR_xres(), 12, color_dark_gray);
      RvR_render_string(2, 2, 1, "Go to: ", color_white);
      RvR_render_string(35, 2, 1, menu_input, color_white);
   }

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}
//-------------------------------------
