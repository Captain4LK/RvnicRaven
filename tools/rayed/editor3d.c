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
#include <string.h>

#include "RvR/RvR.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "color.h"
#include "texture.h"
#include "draw.h"
#include "map.h"
#include "editor.h"
#include "editor3d.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int16_t wx = 0;
static int16_t wy = 0;
static int wlocation = 0;
static int menu = 0;

static uint16_t texture_selected = 0;
static int texture_selection_scroll = 0;
static int brush = 0;
static char menu_input[512] = {0};
//-------------------------------------

//Function prototypes
static void mouse_world_pos(int mx, int my, int16_t *x, int16_t *y, int *location);
void floor_height_flood_fill(uint16_t ftex, uint16_t ctex, RvR_fix16 fheight, RvR_fix16 cheight, int x, int y, RvR_fix16 theight);

static Map_sprite *sprite_selected();
//-------------------------------------

//Function implementations

void editor3d_update()
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

      if(RvR_key_pressed(RVR_KEY_R))
      {
         if(wlocation==4&&sprite_selec!=NULL)
            sprite_selec->flags ^= 8;
      }

      if(RvR_key_pressed(RVR_KEY_T)&&wlocation==4&&sprite_selec!=NULL)
      {
         int flag = (sprite_selec->flags & 96) >> 5;
         sprite_selec->flags ^= flag << 5;
         flag = (flag + 1) % 3;
         sprite_selec->flags |= flag << 5;
      }

      if(RvR_key_pressed(RVR_KEY_1)&&wlocation==4&&sprite_selec!=NULL)
         sprite_selec->flags^=128;

      if(RvR_key_pressed(RVR_KEY_F)&&wlocation==4&&sprite_selec!=NULL)
      {
         int flag = (sprite_selec->flags&6)/2;
         flag = (flag+1)&3;
         sprite_selec->flags&=~(uint32_t)6;
         sprite_selec->flags|=flag*2;
      }

      if(RvR_key_pressed(RVR_KEY_PGUP))
      {
         if(wlocation==0||wlocation==2)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_floor(wx, wy, 1);
            else
               editor_ed_floor(wx, wy, 1);
         }
         if(wlocation==1||wlocation==3)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_ceiling(wx, wy, 1);
            else
               editor_ed_ceiling(wx, wy, 1);
         }
         if(wlocation==4&&sprite_selec!=NULL)
            sprite_selec->z += 64*64;
      }
      else if(RvR_key_pressed(RVR_KEY_PGDN))
      {
         if(wlocation==0||wlocation==2)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_floor(wx, wy, -1);
            else
               editor_ed_floor(wx, wy, -1);
         }
         if(wlocation==1||wlocation==3)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_ceiling(wx, wy, -1);
            else
               editor_ed_ceiling(wx, wy, -1);
         }
         if(wlocation==4&&sprite_selec!=NULL)
            sprite_selec->z -= 64*64;
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
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_floor_tex(wx, wy, texture_selected);
            else
               editor_ed_floor_tex(wx, wy, texture_selected);
         }
         else if(wlocation==1)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_ceiling_tex(wx, wy, texture_selected);
            else
               editor_ed_ceiling_tex(wx, wy, texture_selected);
         }
         else if(wlocation==2)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_floor_wall_tex(wx, wy, texture_selected);
            else
               editor_ed_floor_wall_tex(wx, wy, texture_selected);
         }
         else if(wlocation==3)
         {
            if(RvR_key_down(RVR_KEY_LSHIFT))
               editor_ed_flood_ceiling_wall_tex(wx, wy, texture_selected);
            else
               editor_ed_ceiling_wall_tex(wx, wy, texture_selected);
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

      int tex_per_row = (RvR_xres())/64;
      int tex_per_col = (RvR_yres())/64;
      int index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
      int set_pos = 0;
      if(RvR_key_pressed(RVR_KEY_UP))
      {
         index-=tex_per_row;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;
         if(my<0)
         {
            texture_selection_scroll--;
            my+=64;
         }

         set_pos = 1;
      }
      if(RvR_key_pressed(RVR_KEY_DOWN))
      {
         index+=tex_per_row;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;
         if(my>tex_per_col*64)
         {
            texture_selection_scroll++;
            my-=64;
         }

         set_pos = 1;
      }
      if(RvR_key_pressed(RVR_KEY_LEFT))
      {
         index--;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;

         set_pos = 1;
      }
      if(RvR_key_pressed(RVR_KEY_RIGHT))
      {
         index++;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;

         set_pos = 1;
      }

      if(index<0)
      {
         texture_selection_scroll = 0;
         RvR_mouse_set_pos(32,32);
         mx = 32;
         my = 32;
         index = 0;
         set_pos = 0;
      }

      if(set_pos)
         RvR_mouse_set_pos(mx,my);

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
            map_sky_tex_set(texture_list_used.data[index]);
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

      int tex_per_row = (RvR_xres())/64;
      int tex_per_col = (RvR_yres())/64;
      int index = mx / 64 + (texture_selection_scroll + my / 64) * RvR_xres() / 64;
      int set_pos = 0;
      if(RvR_key_pressed(RVR_KEY_UP))
      {
         index-=tex_per_row;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;
         if(my<0)
         {
            texture_selection_scroll--;
            my+=64;
         }

         set_pos = 1;
      }
      if(RvR_key_pressed(RVR_KEY_DOWN))
      {
         index+=tex_per_row;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;
         if(my>tex_per_col*64)
         {
            texture_selection_scroll++;
            my-=64;
         }

         set_pos = 1;
      }
      if(RvR_key_pressed(RVR_KEY_LEFT))
      {
         index--;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;

         set_pos = 1;
      }
      if(RvR_key_pressed(RVR_KEY_RIGHT))
      {
         index++;
         mx = (index%tex_per_row)*64+32;
         my = (index/tex_per_row)*64+32-texture_selection_scroll*64;

         set_pos = 1;
      }

      if(index<0)
      {
         texture_selection_scroll = 0;
         RvR_mouse_set_pos(32,32);
         mx = 32;
         my = 32;
         index = 0;
         set_pos = 0;
      }

      if(set_pos)
         RvR_mouse_set_pos(mx,my);

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
               map_sky_tex_set(texture_list.data[index]);
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

         int selection = strtol(menu_input,NULL,10);
         //TODO: What we should do: binary search
         //What I did: slow crap
         for(int i = 0;i<texture_list.data_used;i++)
         {
            if(texture_list.data[i]>=selection)
            {
               int tex_per_row = (RvR_xres())/64;
               int tex_per_col = (RvR_yres())/64;
               texture_selection_scroll = i/RvR_non_zero(tex_per_row);
               mx = (i%tex_per_row)*64+32;
               my = (i/tex_per_row)*64+32-texture_selection_scroll*64;
               RvR_mouse_set_pos(mx,my);
               break;
            }
         }
      }
   }
}

void editor3d_draw()
{
   int mx, my;
   RvR_mouse_pos(&mx, &my);

   if(menu==0)
   {
      //Highlight selected tile
      static uint16_t texture_highlight_old = 65535;
      uint16_t texture_highlight = 0;

      if(wlocation==0)
         texture_highlight = RvR_ray_map_floor_tex_at(map,wx, wy);
      if(wlocation==1)
         texture_highlight = RvR_ray_map_ceil_tex_at(map,wx, wy);
      if(wlocation==2)
         texture_highlight = RvR_ray_map_wall_ftex_at(map,wx, wy);
      if(wlocation==3)
         texture_highlight = RvR_ray_map_wall_ctex_at(map,wx, wy);

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

         if(wlocation==0)
            RvR_ray_map_floor_tex_set(map,wx, wy, UINT16_MAX - 1);
         if(wlocation==1)
            RvR_ray_map_ceil_tex_set(map,wx, wy, UINT16_MAX - 1);
         if(wlocation==2)
            RvR_ray_map_wall_ftex_set(map,wx, wy, UINT16_MAX - 1);
         if(wlocation==3)
            RvR_ray_map_wall_ctex_set(map,wx, wy, UINT16_MAX - 1);
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

         if(wlocation==0)
            RvR_ray_map_floor_tex_set(map,wx, wy, UINT16_MAX - 2);
         if(wlocation==1)
            RvR_ray_map_ceil_tex_set(map,wx, wy, UINT16_MAX - 2);
         if(wlocation==2)
            RvR_ray_map_wall_ftex_set(map,wx, wy, UINT16_MAX - 2);
         if(wlocation==3)
            RvR_ray_map_wall_ctex_set(map,wx, wy, UINT16_MAX - 2);
      }
      //-------------------------------------

      RvR_ray_draw_begin();
      Map_sprite *s = map_sprites;
      while(s!=NULL)
      {
         //TODO
         RvR_ray_draw_sprite(&camera,s->x,s->y,s->z, s->direction, s->texture, s->flags);
         s = s->next;
      }

      RvR_ray_draw_map(&camera,map);
      RvR_ray_draw_end(&camera,map);

      texture_highlight_old = texture_highlight;
      if(wlocation==0)
         RvR_ray_map_floor_tex_set(map,wx, wy, texture_highlight);
      if(wlocation==1)
         RvR_ray_map_ceil_tex_set(map,wx, wy, texture_highlight);
      if(wlocation==2)
         RvR_ray_map_wall_ftex_set(map,wx, wy, texture_highlight);
      if(wlocation==3)
         RvR_ray_map_wall_ctex_set(map,wx, wy, texture_highlight);

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
            snprintf(tmp_font,16,"%d",texture_list_used.data[index]);
            RvR_render_rectangle_fill(x*64,y*64,strlen(tmp_font)*4+1,7,color_black);
            RvR_render_string(x*64+1,y*64+1,1,tmp_font,color_yellow);
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
               snprintf(tmp_font,16,"%d",texture_list.data[index]);
               RvR_render_rectangle_fill(x*64,y*64,strlen(tmp_font)*4+1,7,color_black);
               RvR_render_string(x*64+1,y*64+1,1,tmp_font,color_yellow);
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
      RvR_render_string(2,2,1,"Go to: ",color_white);
      RvR_render_string(35,2,1,menu_input,color_white);
   }

   //Draw cursor
   RvR_render_horizontal_line(mx - 4, mx - 1, my, color_magenta);
   RvR_render_horizontal_line(mx + 1, mx + 4, my, color_magenta);
   RvR_render_vertical_line(mx, my - 1, my - 4, color_magenta);
   RvR_render_vertical_line(mx, my + 1, my + 4, color_magenta);
}

static void mouse_world_pos(int mx, int my, int16_t *x, int16_t *y, int *location)
{
   if(my<0||my>=RvR_yres()||mx<0||mx>=RvR_xres())
      return;

   RvR_fix16 dir0x = RvR_fix16_cos(camera.dir-camera.fov/2);
   RvR_fix16 dir0y = RvR_fix16_sin(camera.dir-camera.fov/2);
   RvR_fix16 dir1x = RvR_fix16_cos(camera.dir+camera.fov/2);
   RvR_fix16 dir1y = RvR_fix16_sin(camera.dir+camera.fov/2);
   RvR_fix16 ray_start_floor_height = RvR_ray_map_floor_height_at(map,camera.x / 65536, camera.y / 65536) -  camera.z;
   RvR_fix16 ray_start_ceil_height = RvR_ray_map_ceiling_height_at(map,camera.x / 65536, camera.y / 65536) - camera.z;
   int32_t ray_middle_row = (RvR_yres() / 2) + camera.shear;
   RvR_fix16 cos = RvR_non_zero(RvR_fix16_cos(camera.fov / 2));
   dir0x = RvR_fix16_div(dir0x,cos);
   dir0y = RvR_fix16_div(dir0y,cos);
   dir1x = RvR_fix16_div(dir1x,cos);
   dir1y = RvR_fix16_div(dir1y,cos);

   RvR_fix16 dx = dir1x - dir0x;
   RvR_fix16 dy = dir1y - dir0y;

   RvR_ray_hit_result hits[32] = {0};
   int hit_count = 0;

   RvR_ray r;
   r.x = camera.x;
   r.y = camera.y;

   RvR_fix16 current_dx = 0;
   RvR_fix16 current_dy = 0;
   current_dx += dx * mx;
   current_dy += dy * mx;

   r.dirx = dir0x + (current_dx / RvR_xres());
   r.diry = dir0y + (current_dy / RvR_xres());

   RvR_ray_cast_multi_hit(map,r, hits, &hit_count,32);

   //last written Y position, can never go backwards
   RvR_fix16 f_pos_y = RvR_yres();
   RvR_fix16 c_pos_y = -1;

   //world coordinates (relative to camera height though)
   RvR_fix16 f_z1_world = ray_start_floor_height;
   RvR_fix16 c_z1_world = ray_start_ceil_height;

   int start = 0;
   int end = 0;
   const int direction_modx[4] = {0, 1, 0, -1};
   const int direction_mody[4] = {1, 0, -1, 0};
   RvR_fix16 fov_factor_x = RvR_fix16_tan(camera.fov / 2);
   RvR_fix16 fov_factor_y = (RvR_yres() * fov_factor_x) / (RvR_xres() / 2);

   //we'll be simulatenously drawing the floor and the ceiling now
   for(RvR_fix16 j = 0; j<32; ++j)
   {                    //^ = add extra iteration for horizon plane
      int8_t drawing_horizon = j==(32- 1);

      RvR_ray_hit_result hit;
      RvR_fix16 distance = 1;

      RvR_fix16 f_wall_height = 0, c_wall_height = 0;
      RvR_fix16 f_z2_world = 0,    c_z2_world = 0;
      RvR_fix16 f_z1_screen = 0,   c_z1_screen = 0;
      RvR_fix16 f_z2_screen = 0,   c_z2_screen = 0;

      if(!drawing_horizon)
      {
         hit = hits[j];
         distance = RvR_non_zero(hit.distance);
         //p.hit = hit;

         f_wall_height = RvR_ray_map_floor_height_at(map,hit.squarex, hit.squarey);
         f_z2_world = f_wall_height - camera.z;
         f_z1_screen = ray_middle_row - (f_z1_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fov_factor_y,distance));
         f_z2_screen = ray_middle_row - (f_z2_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fov_factor_y,distance));

         c_wall_height = RvR_ray_map_ceiling_height_at(map,hit.squarex, hit.squarey);
         c_z2_world = c_wall_height - camera.z;
         c_z1_screen = ray_middle_row - (c_z1_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fov_factor_y,distance));
         c_z2_screen = ray_middle_row - (c_z2_world*RvR_yres())/RvR_non_zero(RvR_fix16_mul(fov_factor_y,distance));
      }
      else
      {
         f_z1_screen = ray_middle_row;
         c_z1_screen = ray_middle_row + 1;
         hit.squarex = -1;
         hit.squarey = -1;
         hit.direction = 0;
      }

      RvR_fix16 limit;

      //floor until wall
      limit = RvR_clamp(f_z1_screen, c_pos_y + 1, RvR_yres());
      start = limit;
      end = f_pos_y - 1;
      if(my>=start&&my<=end)
      {
         *x = hit.squarex + direction_modx[hit.direction];
         *y = hit.squarey + direction_mody[hit.direction];
         *location = 0;

         return;
      }

      if(f_pos_y>limit)
         f_pos_y = limit;

      //ceiling until wall
      limit = RvR_clamp(c_z1_screen, -1, f_pos_y - 1);
      start = c_pos_y + 1;
      end = limit;
      if(my>=start&&my<=end)
      {
         *x = hit.squarex + direction_modx[hit.direction];
         *y = hit.squarey + direction_mody[hit.direction];
         *location = 1;

         return;
      }

      if(c_pos_y<limit)
         c_pos_y = limit;

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         //floor wall
         if(f_pos_y>0&&f_z1_world!=f_z2_world)  //still pixels left?
         {
            limit = RvR_clamp(f_z2_screen, c_pos_y + 1, RvR_yres());
            start = limit;
            end = f_pos_y - 1;
            if(my>=start&&my<=end)
            {
               *x = hit.squarex;
               *y = hit.squarey;
               *location = 2;

               return;
            }

            if(f_pos_y>limit)
               f_pos_y = limit;

            f_z1_world = f_z2_world; //for the next iteration
         }               //^ purposfully allow outside screen bounds here

         //draw ceiling wall
         if(c_pos_y<RvR_yres() - 1&&c_z1_world!=c_z2_world) //pixels left?
         {
            limit = RvR_clamp(c_z2_screen, -1, f_pos_y - 1);
            start = c_pos_y + 1;
            end = limit;
            if(my>=start&&my<=end)
            {
               *x = hit.squarex;
               *y = hit.squarey;
               *location = 3;

               return;
            }

            if(c_pos_y<limit)
               c_pos_y = limit;

            c_z1_world = c_z2_world; //for the next iteration
         }              //^ puposfully allow outside screen bounds here
      }
   }
}

void floor_height_flood_fill(uint16_t ftex, uint16_t ctex, RvR_fix16 fheight, RvR_fix16 cheight, int x, int y, RvR_fix16 theight)
{
   if(map_tile_comp(ftex, ctex, fheight, cheight, x, y))
   {
      RvR_ray_map_floor_height_set(map,x, y, theight);

      floor_height_flood_fill(ftex, ctex, fheight, cheight, x - 1, y, theight);
      floor_height_flood_fill(ftex, ctex, fheight, cheight, x + 1, y, theight);
      floor_height_flood_fill(ftex, ctex, fheight, cheight, x, y - 1, theight);
      floor_height_flood_fill(ftex, ctex, fheight, cheight, x, y + 1, theight);
   }
}

static Map_sprite *sprite_selected()
{
   int mx, my;
   Map_sprite *min = NULL;
   RvR_fix16 depth_min = INT32_MAX;
   Map_sprite *sp = map_sprites;
   RvR_mouse_pos(&mx, &my);
   RvR_fix16 cos = RvR_fix16_cos(camera.dir);
   RvR_fix16 sin = RvR_fix16_sin(camera.dir);
   RvR_fix16 fovx = RvR_fix16_tan(camera.fov/2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres()*fovx*2,RvR_xres()<<16);
   RvR_fix16 sin_fov = RvR_fix16_mul(sin,fovx);
   RvR_fix16 cos_fov = RvR_fix16_mul(cos,fovx);
   RvR_fix16 middle_row = (RvR_yres()/2)+camera.shear;

   for(;sp!=NULL;sp = sp->next)
   {
      if(sp->flags&8)
      {
         RvR_texture *tex = RvR_texture_get(sp->texture);

         //Translate sprite to world space
         RvR_fix16 dirx = RvR_fix16_cos(sp->direction);
         RvR_fix16 diry = RvR_fix16_sin(sp->direction);
         RvR_fix16 half_width = (tex->width*65536)/(64*2);
         RvR_fix16 p0x = RvR_fix16_mul(-diry,half_width)+sp->x;
         RvR_fix16 p0y = RvR_fix16_mul(dirx,half_width)+sp->y;
         RvR_fix16 p1x = RvR_fix16_mul(diry,half_width)+sp->x;
         RvR_fix16 p1y = RvR_fix16_mul(-dirx,half_width)+sp->y;

         //Translate to camera space
         RvR_fix16 x0 = p0x-camera.x;
         RvR_fix16 y0 = p0y-camera.y;
         RvR_fix16 x1 = p1x-camera.x;
         RvR_fix16 y1 = p1y-camera.y;
         RvR_fix16 tp0x = RvR_fix16_mul(-x0,sin)+RvR_fix16_mul(y0,cos);
         RvR_fix16 tp0y = RvR_fix16_mul(x0,cos_fov)+RvR_fix16_mul(y0,sin_fov);
         RvR_fix16 tp1x = RvR_fix16_mul(-x1,sin)+RvR_fix16_mul(y1,cos);
         RvR_fix16 tp1y = RvR_fix16_mul(x1,cos_fov)+RvR_fix16_mul(y1,sin_fov);

         //Behind camera
         if(tp0y<-128&&tp1y<-128)
            continue;

         //Sprite not facing camera
         //--> swap p0 and p1 and toggle y-axis mirror flag
         if(RvR_fix16_mul(tp0x,tp1y)-RvR_fix16_mul(tp1x,tp0y)>0)
         {
            RvR_fix16 tmp = tp0x;
            tp0x = tp1x;
            tp1x = tmp;

            tmp = tp0y;
            tp0y = tp1y;
            tp1y = tmp;
            //sp.flags^=2;
         }

         RvR_fix16 sx0;
         RvR_fix16 sz0;
         RvR_fix16 sx1;
         RvR_fix16 sz1;

         //Here we can treat everything as if we have a 90 degree
         //fov, since the rotation to camera space transforms it to
         //that
         //Check if in fov
         //Left point in fov
         if(tp0x>=-tp0y)
         {
            //Sprite completely out of sight
            if(tp0x>tp0y)
               continue;

            sx0 = RvR_min(RvR_xres()*32768+RvR_fix16_div(tp0x*(RvR_xres()/2),tp0y),RvR_xres()*65536);
            sz0 = tp0y;
         }
         //Left point to the left of fov
         else
         {
            //Sprite completely out of sight
            if(tp1x<-tp1y)
               continue;

            sx0 = 0;
            RvR_fix16 dx0 = tp1x-tp0x;
            RvR_fix16 dx1 = tp0x+tp0y;
            sz0 = RvR_fix16_div(RvR_fix16_mul(dx0,dx1),tp1y-tp0y+tp1x-tp0x)-tp0x;
         }

         //Right point in fov
         if(tp1x<=tp1y)
         {
            //sprite completely out of sight
            if(tp1x<-tp1y)
               continue;

            sx1 = RvR_min(RvR_xres()*32768+RvR_fix16_div(tp1x*(RvR_xres()/2),tp1y),RvR_xres()*65536);
            sz1 = tp1y;
         }
         //Right point to the right of fov
         else
         {
            //sprite completely out of sight
            if(tp0x>tp0y)
               continue;

            RvR_fix16 dx0 = tp1x-tp0x;
            RvR_fix16 dx1 = tp0y-tp0x;
            sx1 = RvR_xres()*65536;
            sz1 = tp0x-RvR_fix16_div(RvR_fix16_mul(dx0,dx1),tp1y-tp0y-tp1x+tp0x);
         }

         //Near clip sprite 
         if(sz0<1024||sz1<1024)
            continue;

         //Far clip sprite
         if(sz0>32*65536&&sz1>32*65536)
            continue;

         if(sx0>sx1)
            continue;

         RvR_fix16 sy0 = RvR_fix16_div(sp->z-camera.z,RvR_non_zero(RvR_fix16_mul(fovy,sz0)));
         sy0 = RvR_fix16_mul(RvR_yres()*65536,32768-sy0);
         RvR_fix16 sy1 = RvR_fix16_div(sp->z-camera.z,RvR_non_zero(RvR_fix16_mul(fovy,sz1)));
         sy1 = RvR_fix16_mul(RvR_yres()*65536,32768-sy1);

         RvR_fix16 scale_vertical = tex->height*1024;
         if(mx<(sx0>>16)||mx>=(sx1>>16))
            continue;

         RvR_fix16 cy0 = RvR_fix16_div(RvR_yres()*(sp->z+scale_vertical-camera.z),RvR_fix16_mul(sz0,fovy));
         RvR_fix16 cy1 = RvR_fix16_div(RvR_yres()*(sp->z+scale_vertical-camera.z),RvR_fix16_mul(sz1,fovy));
         cy0 = RvR_yres()*32768-cy0;
         cy1 = RvR_yres()*32768-cy1;
         RvR_fix16 step_cy = RvR_fix16_div(cy1-cy0,RvR_non_zero(sx1-sx0));
         RvR_fix16 cy = cy0;

         RvR_fix16 fy0 = RvR_fix16_div(RvR_yres()*(sp->z-camera.z),RvR_fix16_mul(sz0,fovy));
         RvR_fix16 fy1 = RvR_fix16_div(RvR_yres()*(sp->z-camera.z),RvR_fix16_mul(sz1,fovy));
         fy0 = RvR_yres()*32768-fy0;
         fy1 = RvR_yres()*32768-fy1;
         RvR_fix16 step_fy = RvR_fix16_div(fy1-fy0,RvR_non_zero(sx1-sx0));
         RvR_fix16 fy = fy0;

         RvR_fix16 denom = RvR_fix16_mul(sz1,sz0);
         RvR_fix16 num_step_z = RvR_fix16_div(sz0-sz1,RvR_non_zero(sx1-sx0));
         RvR_fix16 num_z = sz1;

         //Adjust for fractional part
         RvR_fix16 xfrac = sx0-(sx0/65536)*65536;
         cy-=RvR_fix16_mul(xfrac,step_cy);
         fy-=RvR_fix16_mul(xfrac,step_fy);
         num_z-=RvR_fix16_mul(xfrac,num_step_z);

         int x_step = mx-(sx0>>16)-1;
         cy+=step_cy*x_step;
         fy+=step_fy*x_step;
         num_z+=num_step_z*x_step;

         RvR_fix16 depth = RvR_fix16_div(denom,RvR_non_zero(num_z));

         //Clip floor
         int ybot = RvR_yres()-1;
         const RvR_ray_depth_buffer_entry *clip = RvR_ray_depth_buffer_entry_floor(mx);
         while(clip!=NULL)
         {
            if(depth>clip->depth&&ybot>clip->limit)
               ybot = clip->limit;
            clip = clip->next;
         }

         //Clip ceiling
         int ytop = 0;
         clip = RvR_ray_depth_buffer_entry_ceiling(mx);
         while(clip!=NULL)
         {
            if(depth>clip->depth&&ytop<clip->limit)
               ytop = clip->limit;
            clip = clip->next;
         }

         int wy =  ytop;

         //Ceiling
         int y_to = RvR_min(cy>>16,ybot);
         if(y_to>wy)
            wy = y_to;

         y_to = RvR_min((fy>>16)-1,ybot);
         if(my>=wy&&my<=y_to&&depth<depth_min)
         {
            min = sp;
            depth_min = depth;
         }

         continue;
      }

      RvR_ray_pixel_info px = RvR_ray_map_to_screen(&camera,sp->x,sp->y,sp->z);
      if(px.depth<0||px.depth>24 * 65536||px.depth>depth_min)
         continue;

      RvR_texture *texture = RvR_texture_get(sp->texture);
      int mask = (1<<RvR_log2(texture->height))-1;

      RvR_fix16 tpx = sp->x-camera.x;
      RvR_fix16 tpy = sp->y-camera.y;
      RvR_fix16 depth = RvR_fix16_mul(tpx,cos)+RvR_fix16_mul(tpy,sin);
      tpx = RvR_fix16_mul(tpx,sin)-RvR_fix16_mul(tpy,cos);

      //Dimensions
      RvR_fix16 top = middle_row*65536-RvR_fix16_div(RvR_yres()*(sp->z-camera.z+texture->height*16*64),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
      int y0 = (top+65535)/65536;

      RvR_fix16 bot = middle_row*65536-RvR_fix16_div(RvR_yres()*(sp->z-camera.z),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
      int y1 = (bot-1)/65536;

      RvR_fix16 left = RvR_xres()*32768-RvR_fix16_div((RvR_xres()/2)*(tpx+texture->width*8*64),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
      int x0 = (left+65535)/65536;

      RvR_fix16 right = RvR_xres()*32768-RvR_fix16_div((RvR_xres()/2)*(tpx-texture->width*8*64),RvR_non_zero(RvR_fix16_mul(depth,fovx)));
      int x1 = (right-1)/65536;

      //Floor and ceiling clip
      RvR_fix16 cy = middle_row*65536-RvR_fix16_div(RvR_yres()*(RvR_ray_map_floor_height_at(map,sp->x/65536,sp->y/65536)-camera.z),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
      int clip_bottom = RvR_min(cy/65536,RvR_yres());

      cy = middle_row*65536-RvR_fix16_div(RvR_yres()*(RvR_ray_map_ceiling_height_at(map,sp->x/65536,sp->y/65536)-camera.z),RvR_non_zero(RvR_fix16_mul(depth,fovy)));
      int clip_top = RvR_max(cy/65536,0);

      y0 = RvR_max(y0,clip_top);
      y1 = RvR_min(y1,clip_bottom);
      x1 = RvR_min(x1,RvR_xres());
      RvR_fix16 step_v = RvR_fix16_mul(fovy,depth)/RvR_yres();
      RvR_fix16 step_u = RvR_fix16_mul(2*fovx,depth)/RvR_xres();
      RvR_fix16 u = RvR_fix16_mul(step_u,x0*65536-left);

      if(my<y0||my>=y1||mx<x0||mx>=x1)
         continue;

      int x = mx;
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      const RvR_ray_depth_buffer_entry *clip = RvR_ray_depth_buffer_entry_floor(x);
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = RvR_ray_depth_buffer_entry_ceiling(x);
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      if(my>ys&&my<ye)
      {
         min = sp;
         depth_min = px.depth;
      }
   }

   return min;
}
//-------------------------------------
