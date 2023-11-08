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
static void mouse_world_pos(int mx, int my, int16_t *x, int16_t *y, int *location);

static Map_sprite *sprite_selected();
//-------------------------------------

//Function implementations

void editor3d_update(void)
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(menu==0)
   {
      camera_update();

      //Get real world tile position of mouse
      if(!RvR_key_down(RVR_KEY_LCTRL))
      {
         mouse_world_pos(mx, my, &wx, &wy, &wlocation);
      }

      Map_sprite *sprite_selec = sprite_selected();
      if(sprite_selec!=NULL)
         wlocation = 4;
      else if(RvR_key_down(RVR_KEY_LCTRL)&&wlocation==4)
      {
         wlocation = 0;
         wx = -1;
         wy = -1;
      }

      //TODO: having this would be usefull but the keys are taken
      //think of different keybindings
      /*if(RvR_key_down(RVR_KEY_1))
         wlocation = 0;
      else if(RvR_key_down(RVR_KEY_2))
         wlocation = 1;
      else if(RvR_key_down(RVR_KEY_3))
         wlocation = 2;
      else if(RvR_key_down(RVR_KEY_4))
         wlocation = 3;*/

      if(RvR_key_pressed(RVR_KEY_R)&&wlocation==4&&sprite_selec!=NULL)
      {
         int flag = (sprite_selec->flags & 24) >> 3;
         sprite_selec->flags ^= flag << 3;
         flag = (flag + 1) % 3;
         sprite_selec->flags |= flag << 3;
      }

      if(RvR_key_pressed(RVR_KEY_T)&&wlocation==4&&sprite_selec!=NULL)
      {
         int flag = (sprite_selec->flags & 96) >> 5;
         sprite_selec->flags ^= flag << 5;
         flag = (flag + 1) % 3;
         sprite_selec->flags |= flag << 5;
      }

      else if(RvR_key_pressed(RVR_KEY_PERIOD)&&wlocation==4&&sprite_selec!=NULL)
         sprite_selec->direction -= RvR_key_down(RVR_KEY_LSHIFT)?32 * 16:8 * 16;
      else if(RvR_key_pressed(RVR_KEY_COMMA)&&wlocation==4&&sprite_selec!=NULL)
         sprite_selec->direction += RvR_key_down(RVR_KEY_LSHIFT)?32 * 16:8 * 16;

      if(RvR_key_pressed(RVR_KEY_1)&&wlocation==4&&sprite_selec!=NULL)
         sprite_selec->flags ^= 128;

      if(RvR_key_pressed(RVR_KEY_F)&&wlocation==4&&sprite_selec!=NULL)
      {
         int flag = (sprite_selec->flags & 6) / 2;
         flag = (flag + 1) & 3;
         sprite_selec->flags &= ~(uint32_t)6;
         sprite_selec->flags |= flag * 2;
      }

      if(RvR_key_pressed(RVR_KEY_PGUP))
      {
         if(wlocation==0||wlocation==2)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_floor(wx, wy, 1);
            //else
               //editor_ed_floor(wx, wy, 1);
         }
         if(wlocation==1||wlocation==3)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_ceiling(wx, wy, 1);
            //else
               //editor_ed_ceiling(wx, wy, 1);
         }
         if(wlocation==4&&sprite_selec!=NULL)
            sprite_selec->z += 64 * 64;
      }
      else if(RvR_key_pressed(RVR_KEY_PGDN))
      {
         if(wlocation==0||wlocation==2)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_floor(wx, wy, -1);
            //else
               //editor_ed_floor(wx, wy, -1);
         }
         if(wlocation==1||wlocation==3)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_ceiling(wx, wy, -1);
            //else
               //editor_ed_ceiling(wx, wy, -1);
         }
         if(wlocation==4&&sprite_selec!=NULL)
            sprite_selec->z -= 64 * 64;
      }

      //To prevent accidental texture editing after selecting a texture
      if(RvR_key_pressed(RVR_BUTTON_LEFT))
      {
         if(wlocation==4&&sprite_selec!=NULL)
            sprite_selec->texture = texture_selected;
         else
            brush = 1;
      }
      if(RvR_key_released(RVR_BUTTON_LEFT))
         brush = 0;
      if(brush)
      {
         if(wlocation==0)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_floor_tex(wx, wy, texture_selected);
            //else
               //editor_ed_floor_tex(wx, wy, texture_selected);
         }
         else if(wlocation==1)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_ceiling_tex(wx, wy, texture_selected);
            //else
               //editor_ed_ceiling_tex(wx, wy, texture_selected);
         }
         else if(wlocation==2)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_floor_wall_tex(wx, wy, texture_selected);
            //else
               //editor_ed_floor_wall_tex(wx, wy, texture_selected);
         }
         else if(wlocation==3)
         {
            //if(RvR_key_down(RVR_KEY_LSHIFT))
               //editor_ed_flood_ceiling_wall_tex(wx, wy, texture_selected);
            //else
               //editor_ed_ceiling_wall_tex(wx, wy, texture_selected);
         }
      }

      if(RvR_key_pressed(RVR_KEY_V))
      {
         menu = 1;
         texture_selection_scroll = 0;
      }
   }
   else if(menu==1)
   {
      if(RvR_key_pressed(RVR_KEY_V))
      {
         menu = 2;
         texture_selection_scroll = 0;
      }

      if(RvR_key_pressed(RVR_KEY_ESCAPE))
         menu = 0;

      if(RvR_key_pressed(RVR_KEY_G))
      {
         menu = 3;
         menu_input[0] = '\0';
         RvR_text_input_start(menu_input, 64);
      }

      //texture_selection_scroll += RvR_mouse_wheel_scroll() * -3;
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
            int index = texture_list_used_wrap(texture_list_used.data_last - (mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64));
            texture_selected = texture_list_used.data[index];
            texture_list_used_add(texture_selected);
            menu = 0;
            brush = 0;
         }
         if(RvR_key_pressed(RVR_KEY_S))
         {
            int index = texture_list_used_wrap(texture_list_used.data_last - (mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64));
            //map_sky_tex_set(texture_list_used.data[index]);
            menu = 0;
            brush = 0;
         }
      }
   }
   else if(menu==2)
   {
      if(RvR_key_pressed(RVR_KEY_ESCAPE))
         menu = 0;

      if(RvR_key_pressed(RVR_KEY_G))
      {
         menu = 4;
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
            unsigned index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
            if(index<texture_list.data_used)
            {
               texture_selected = texture_list.data[index];
               texture_list_used_add(texture_selected);
               menu = 0;
               brush = 0;
            }
         }
         if(RvR_key_pressed(RVR_KEY_S))
         {
            unsigned index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
            if(index<texture_list.data_used)
            {
               //map_sky_tex_set(texture_list.data[index]);
               menu = 0;
               brush = 0;
            }
         }
      }
   }

   if(menu==3||menu==4)
   {
      if(RvR_key_pressed(RVR_KEY_ENTER))
      {
         RvR_text_input_end();
         menu = 2;

         int selection = strtol(menu_input, NULL, 10);
         //TODO: What we should do: binary search
         //What I did: slow crap
         for(int i = 0; i<texture_list.data_used; i++)
         {
            if(texture_list.data[i]>=selection)
            {
               int tex_per_row = (RvR_xres()) / 64;
               int tex_per_col = (RvR_yres()) / 64;
               texture_selection_scroll = i / RvR_non_zero(tex_per_row);
               mx = (i % tex_per_row) * 64 + 32;
               my = (i / tex_per_row) * 64 + 32 - texture_selection_scroll * 64;
               RvR_mouse_set_pos(mx, my);
               break;
            }
         }
      }
   }
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

static void mouse_world_pos(int mx, int my, int16_t *x, int16_t *y, int *location)
{
}

static Map_sprite *sprite_selected()
{
   return NULL;
}
//-------------------------------------
