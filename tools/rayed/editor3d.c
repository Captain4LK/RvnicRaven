/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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

      if(RvR_key_down(RVR_KEY_1))
         wlocation = 0;
      else if(RvR_key_down(RVR_KEY_2))
         wlocation = 1;
      else if(RvR_key_down(RVR_KEY_3))
         wlocation = 2;
      else if(RvR_key_down(RVR_KEY_4))
         wlocation = 3;

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
            sprite_selec->z += 64;
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
            sprite_selec->z -= 64;
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

      texture_selection_scroll += RvR_mouse_wheel_scroll() * -4;

      if(mx / 64<RvR_xres() / 64)
      {
         if(RvR_key_pressed(RVR_BUTTON_LEFT))
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

      texture_selection_scroll += RvR_mouse_wheel_scroll() * -4;

      if(mx / 64<RvR_xres() / 64)
      {
         if(RvR_key_pressed(RVR_BUTTON_LEFT))
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
         //RvR_ray_draw_sprite(s->pos, s->direction, s->texture, s->flags);
         s = s->next;
      }

      RvR_ray_draw_map(&camera,map);
      RvR_ray_draw_end();

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
   else if(menu==1)
   {
      RvR_render_clear(color_black);

      for(int y = 0; y<RvR_yres() / 64; y++)
      {
         for(int x = 0; x<RvR_xres() / 64; x++)
         {
            int index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
            index = texture_list_used_wrap(texture_list_used.data_last - index);
            draw_fit64(x * 64, y * 64, texture_list_used.data[index]);
         }
      }

      if(mx / 64<RvR_xres() / 64)
         RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);
   }
   else if(menu==2)
   {
      RvR_render_clear(color_black);

      for(int y = 0; y<RvR_yres() / 64; y++)
      {
         for(int x = 0; x<RvR_xres() / 64; x++)
         {
            unsigned index = texture_selection_scroll * (RvR_xres() / 64) + y * (RvR_xres() / 64) + x;
            if(index<texture_list.data_used)
               draw_fit64(x * 64, y * 64, texture_list.data[index]);
         }
      }

      if(mx / 64<RvR_xres() / 64)
         RvR_render_rectangle((mx / 64) * 64, (my / 64) * 64, 64, 64, color_white);
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
   //RvR_fix22_vec2 dir0 = RvR_fix22_vec2_rot(RvR_ray_get_angle() - (RvR_ray_get_fov() / 2));
   //RvR_fix22_vec2 dir1 = RvR_fix22_vec2_rot(RvR_ray_get_angle() + (RvR_ray_get_fov() / 2));
   //RvR_fix22 ray_start_floor_height = RvR_ray_map_floor_height_at(RvR_div_round_down(RvR_ray_get_position().x,1024),RvR_div_round_down(RvR_ray_get_position().y,1024))-1*RvR_ray_get_position().z;
   //RvR_fix22 ray_start_ceil_height = RvR_ray_map_ceiling_height_at(RvR_div_round_down(RvR_ray_get_position().x,1024),RvR_div_round_down(RvR_ray_get_position().y,1024))-1*RvR_ray_get_position().z;
   RvR_fix16 ray_start_floor_height = RvR_ray_map_floor_height_at(map,camera.x / 65536, camera.y / 65536) - 1 * camera.z;
   RvR_fix16 ray_start_ceil_height = RvR_ray_map_ceiling_height_at(map,camera.x / 65536, camera.y / 65536) - 1 * camera.z;
   int32_t ray_middle_row = (RvR_yres() / 2) + camera.shear;
   RvR_fix16 cos = RvR_non_zero(RvR_fix16_cos(camera.fov / 2));
   dir0x = RvR_fix16_div(dir0x,cos);
   dir0y = RvR_fix16_div(dir0y,cos);
   dir1x = RvR_fix16_div(dir1x,cos);
   dir1y = RvR_fix16_div(dir1y,cos);
   //dir0.x = (dir0.x * 1024) / cos;
   //dir0.y = (dir0.y * 1024) / cos;
   //dir1.x = (dir1.x * 1024) / cos;
   //dir1.y = (dir1.y * 1024) / cos;

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
         f_z1_screen = ray_middle_row - (f_z1_world*RvR_yres())/RvR_fix16_mul(fov_factor_y,distance);
         f_z2_screen = ray_middle_row - (f_z2_world*RvR_yres())/RvR_fix16_mul(fov_factor_y,distance);
         //f_z1_screen = ray_middle_row - ((f_z1_world * RvR_yres()) / RvR_non_zero((fov_factor_y * distance) / 1024));
         //f_z2_screen = ray_middle_row - ((f_z2_world * RvR_yres()) / RvR_non_zero((fov_factor_y * distance) / 1024));

         c_wall_height = RvR_ray_map_ceiling_height_at(map,hit.squarex, hit.squarey);
         c_z2_world = c_wall_height - camera.z;
         c_z1_screen = ray_middle_row - (c_z1_world*RvR_yres())/RvR_fix16_mul(fov_factor_y,distance);
         c_z2_screen = ray_middle_row - (c_z2_world*RvR_yres())/RvR_fix16_mul(fov_factor_y,distance);
         //c_z1_screen = ray_middle_row - ((c_z1_world * RvR_yres()) / RvR_non_zero((fov_factor_y * distance) / 1024));
         //c_z2_screen = ray_middle_row - ((c_z2_world * RvR_yres()) / RvR_non_zero((fov_factor_y * distance) / 1024));
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
   RvR_fix16 fov_factor_x = RvR_fix16_tan(camera.fov / 2);
   RvR_fix16 fov_factor_y = (RvR_yres() * fov_factor_x) / (RvR_xres() / 2);

   while(sp!=NULL)
   {
      RvR_ray_pixel_info px = RvR_ray_map_to_screen(&camera,sp->x,sp->y,sp->z);

      if(px.depth<0||px.depth>24 * 1024||px.x<-2 * RvR_xres()||px.x>4 * RvR_xres()||px.depth>depth_min)
         goto next;

      RvR_texture *texture = RvR_texture_get(sp->texture);
      RvR_fix16 scale_vertical = RvR_yres() * (texture->height * 16*64);
      RvR_fix16 scale_horizontal = (RvR_xres() / 2) * (texture->width * 16*64);
      int size_vertical = scale_vertical / RvR_non_zero((fov_factor_y * px.depth) / 65536);
      int size_horizontal = scale_horizontal / RvR_non_zero((fov_factor_x * px.depth) / 65536);

      //Reject based on non-clipped bounding rect
      if(mx<px.x - size_horizontal / 2||mx>px.x + size_horizontal / 2)
         goto next;
      if(my<px.y - size_vertical||my>px.y)
         goto next;

      //Clip specific part of sprite
      int y = px.y - size_vertical;
      int sy = 0;
      int ey = size_vertical;

      //Floor and ceiling clip
      //RvR_fix22_vec3 floor_wpos;
      RvR_fix16 floor_wposx = sp->x;
      RvR_fix16 floor_wposy = sp->y;
      RvR_fix16 floor_wposz = RvR_ray_map_floor_height_at(map,sp->x / 65536, sp->y / 65536);
      int clip_bottom = RvR_ray_map_to_screen(&camera,floor_wposx,floor_wposy,floor_wposz).y;
      floor_wposz = RvR_ray_map_ceiling_height_at(map,sp->x / 65536, sp->y / 65536);
      int clip_top = RvR_ray_map_to_screen(&camera,floor_wposx,floor_wposy,floor_wposz).y;
      clip_bottom = clip_bottom>RvR_yres()?RvR_yres():clip_bottom;
      clip_top = clip_top<0?0:clip_top;

      //Clip coordinates to screen/clip_top and clip_bottom
      //if(x<0)
      //sx = -x;
      if(y<clip_top)
         sy = clip_top - y;
      //if(x+ex>RvR_xres())
      //ex = size_horizontal+(RvR_xres()-x-ex);
      if(y + ey>clip_bottom)
         ey = size_vertical + (clip_bottom - y - ey);
      //x = x<0?0:x;
      y = y<clip_top?clip_top:y;

      //Clip against walls
      int ey1 = ey;
      int ys = y;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = RvR_ray_draw_depth_buffer()->floor[mx];
      while(clip!=NULL)
      {
         if(px.depth>clip->depth&&y + (ey1 - sy)>clip->limit)
            ey1 = clip->limit - y + sy;
         clip = clip->next;
      }

      //Clip ceiling
      clip = RvR_ray_draw_depth_buffer()->ceiling[mx];
      while(clip!=NULL)
      {
         if(px.depth>clip->depth&&ys<clip->limit)
         {
            int diff = ys - clip->limit;
            ys = clip->limit;
            ey1 += diff;
         }
         clip = clip->next;
      }

      if(my>sy&&my<ys + ey1)
      {
         min = sp;
         depth_min = px.depth;
      }

next:
      sp = sp->next;
   }

   return min;
}
//-------------------------------------
