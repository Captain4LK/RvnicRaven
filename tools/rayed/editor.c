/*
RvnicRaven retro game engine

Written in 2021,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "map.h"
#include "texture.h"
#include "editor.h"
#include "editor2d.h"
#include "editor3d.h"
//-------------------------------------

//#defines
#define CAMERA_SHEAR_MAX_PIXELS ((CAMERA_SHEAR_MAX * RvR_yres()) / 65536)
#define CAMERA_SHEAR_STEP_FRAME ((RvR_yres() * CAMERA_SHEAR_SPEED) / (RvR_fps() * 4))

#define WRAP(p) ((p) & (UNDO_BUFFER_SIZE - 1))
#define UNDO_RECORD (UINT16_MAX)
#define REDO_RECORD (UINT16_MAX - 2)
#define JUNK_RECORD (UINT16_MAX - 1)
//-------------------------------------

//Typedefs
typedef enum
{
   ED_FLOOR_HEIGHT = 0,
   ED_CEILING_HEIGHT = 1,
   ED_FLOOD_FLOOR_HEIGHT = 2,
   ED_FLOOD_CEILING_HEIGHT = 3,

   ED_FLOOR_TEXTURE = 4,
   ED_CEILING_TEXTURE = 5,
   ED_FLOOR_WALL_TEXTURE = 6,
   ED_CEILING_WALL_TEXTURE = 7,

   ED_FLOOD_FLOOR_TEXTURE = 8,
   ED_FLOOD_CEILING_TEXTURE = 9,
   ED_FLOOD_FLOOR_WALL_TEXTURE = 10,
   ED_FLOOD_CEILING_WALL_TEXTURE = 11,
}Ed_action;
//-------------------------------------

//Variables
RvR_ray_cam camera = {0};

static int editor_mode = 0;

static uint16_t *undo_buffer = NULL;
static int undo_len = 0;
static int undo_pos = 0;
static int redo_len = 0;
static uint32_t undo_entry_len = 0;
//-------------------------------------

//Function prototypes
void camera_update();
static void move_with_collision(RvR_fix16 offx, RvR_fix16 offy, RvR_fix16 offz, int8_t compute_height, int8_t compute_plane, RvR_fix16 *floor_height, RvR_fix16 *ceiling_height);

static void undo_write(uint16_t val);
static void undo_begin(Ed_action action);
static void undo_end();
static int undo_find_end(uint32_t *len);

static void redo_write(uint16_t val);
static int redo_find_end(uint32_t *len);

static void undo_record_height(int16_t x, int16_t y, RvR_fix16 height);
static void redo_record_height(int16_t x, int16_t y, RvR_fix16 height);
static void undo_record_tex(int16_t x, int16_t y, uint16_t tex);
static void redo_record_tex(int16_t x, int16_t y, uint16_t tex);

static void undo_floor_height(int pos, int endpos);
static void redo_floor_height(int pos, int endpos);
static void undo_ceiling_height(int pos, int endpos);
static void redo_ceiling_height(int pos, int endpos);
static void undo_flood_floor_height(int pos, int endpos);
static void redo_flood_floor_height(int pos, int endpos);
static void undo_flood_ceiling_height(int pos, int endpos);
static void redo_flood_ceiling_height(int pos, int endpos);

static void undo_floor_tex(int pos, int endpos);
static void redo_floor_tex(int pos, int endpos);
static void undo_ceiling_tex(int pos, int endpos);
static void redo_ceiling_tex(int pos, int endpos);
static void undo_floor_wall_tex(int pos, int endpos);
static void redo_floor_wall_tex(int pos, int endpos);
static void undo_ceiling_wall_tex(int pos, int endpos);
static void redo_ceiling_wall_tex(int pos, int endpos);

static void undo_flood_floor_tex(int pos, int endpos);
static void redo_flood_floor_tex(int pos, int endpos);
static void undo_flood_ceiling_tex(int pos, int endpos);
static void redo_flood_ceiling_tex(int pos, int endpos);
static void undo_flood_wall_ftex(int pos, int endpos);
static void redo_flood_wall_ftex(int pos, int endpos);
static void undo_flood_wall_ctex(int pos, int endpos);
static void redo_flood_wall_ctex(int pos, int endpos);

static void flood_floor_height(int16_t x, int16_t y, uint16_t ftex, RvR_fix16 height, int fac);
static void flood_ceiling_height(int16_t x, int16_t y, uint16_t ctex, RvR_fix16 height, int fac);
static void flood_floor_tex(int16_t x, int16_t y, uint16_t ftex, uint16_t tex, RvR_fix16 height);
static void flood_ceiling_tex(int16_t x, int16_t y, uint16_t ctex, uint16_t tex, RvR_fix16 height);
static void flood_wall_ftex(int16_t x, int16_t y, uint16_t ftex, uint16_t tex, RvR_fix16 height);
static void flood_wall_ctex(int16_t x, int16_t y, uint16_t ctex, uint16_t tex, RvR_fix16 height);
//-------------------------------------

//Function implementations

void editor_init()
{
   undo_buffer = RvR_malloc(sizeof(*undo_buffer) * UNDO_BUFFER_SIZE, "rayed undo buffer");
   memset(undo_buffer, 0, sizeof(*undo_buffer) * UNDO_BUFFER_SIZE);

   camera.fov = 16384;
   camera.shear = 0;

   RvR_texture_create(UINT16_MAX - 1, 64, 64);
   RvR_texture_create(UINT16_MAX - 2, 64, 128); //No support for texture higher than 128px currently
   //RvR_texture_lock(UINT16_MAX-1);
   //RvR_texture_lock(UINT16_MAX-2);
}

void editor_update()
{
   if(RvR_key_pressed(RVR_KEY_NP_ENTER))
      editor_mode = !editor_mode;

   if(RvR_key_down(RVR_KEY_LCTRL)&&RvR_key_pressed(RVR_KEY_S))
      map_save();

   if(editor_mode==0)
      editor2d_update();
   else
      editor3d_update();
}

void editor_draw()
{
   if(editor_mode==0)
      editor2d_draw();
   else
      editor3d_draw();

   if(RvR_key_pressed(RVR_KEY_U))
      editor_undo();

   if(RvR_key_pressed(RVR_KEY_R))
      editor_redo();

   //RvR_ray_draw_debug(color_white);
}

void editor_undo()
{
   int pos = 0;
   uint32_t len = 0;
   int endpos = undo_find_end(&len);
   if(endpos<0)
      return;
   Ed_action action = undo_buffer[endpos];

   endpos = WRAP(endpos);
   pos = WRAP(undo_pos - 3);

   if(pos==endpos)
      return;

   //New redo entry
   redo_write(REDO_RECORD);
   redo_write(action);

   //Apply undoes
   switch(action)
   {
   case ED_FLOOR_HEIGHT: undo_floor_height(pos, endpos); break;
   case ED_CEILING_HEIGHT: undo_ceiling_height(pos, endpos); break;
   case ED_FLOOD_FLOOR_HEIGHT: undo_flood_floor_height(pos, endpos); break;
   case ED_FLOOD_CEILING_HEIGHT: undo_flood_ceiling_height(pos, endpos); break;
   case ED_FLOOR_TEXTURE: undo_floor_tex(pos, endpos); break;
   case ED_CEILING_TEXTURE: undo_ceiling_tex(pos, endpos); break;
   case ED_FLOOR_WALL_TEXTURE: undo_floor_wall_tex(pos, endpos); break;
   case ED_CEILING_WALL_TEXTURE: undo_ceiling_wall_tex(pos, endpos); break;
   case ED_FLOOD_FLOOR_TEXTURE: undo_flood_floor_tex(pos, endpos); break;
   case ED_FLOOD_CEILING_TEXTURE: undo_flood_ceiling_tex(pos, endpos); break;
   case ED_FLOOD_FLOOR_WALL_TEXTURE: undo_flood_wall_ftex(pos, endpos); break;
   case ED_FLOOD_CEILING_WALL_TEXTURE: undo_flood_wall_ctex(pos, endpos); break;
   }

   redo_write((len >> 16) & UINT16_MAX);
   redo_write(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void editor_redo()
{
   int pos = 0;
   uint32_t len = 0;
   int endpos = redo_find_end(&len);
   if(endpos<0)
      return;

   Ed_action action = undo_buffer[endpos];
   pos = WRAP(undo_pos + 3);

   if(pos==endpos)
      return;

   //New undo entry
   undo_write(UNDO_RECORD);
   undo_write(action);

   //Apply redoes
   switch(action)
   {
   case ED_FLOOR_HEIGHT: redo_floor_height(pos, endpos); break;
   case ED_CEILING_HEIGHT: redo_ceiling_height(pos, endpos); break;
   case ED_FLOOD_FLOOR_HEIGHT: redo_flood_floor_height(pos, endpos); break;
   case ED_FLOOD_CEILING_HEIGHT: redo_flood_ceiling_height(pos, endpos); break;
   case ED_FLOOR_TEXTURE: redo_floor_tex(pos, endpos); break;
   case ED_CEILING_TEXTURE: redo_ceiling_tex(pos, endpos); break;
   case ED_FLOOR_WALL_TEXTURE: redo_floor_wall_tex(pos, endpos); break;
   case ED_CEILING_WALL_TEXTURE: redo_ceiling_wall_tex(pos, endpos); break;
   case ED_FLOOD_FLOOR_TEXTURE: redo_flood_floor_tex(pos, endpos); break;
   case ED_FLOOD_CEILING_TEXTURE: redo_flood_ceiling_tex(pos, endpos); break;
   case ED_FLOOD_FLOOR_WALL_TEXTURE: redo_flood_wall_ftex(pos, endpos); break;
   case ED_FLOOD_CEILING_WALL_TEXTURE: redo_flood_wall_ctex(pos, endpos); break;
   }

   undo_write((len >> 16) & UINT16_MAX);
   undo_write(len & UINT16_MAX);
   undo_buffer[undo_pos] = JUNK_RECORD;
}

void editor_undo_reset()
{
   undo_len = 0;
   undo_pos = 0;
   redo_len = 0;
}

void camera_update()
{
   RvR_fix16 dirx = RvR_fix16_cos(camera.dir) / 8;
   RvR_fix16 diry = RvR_fix16_sin(camera.dir) / 8;
   int speed = 1;
   RvR_fix16 offx = 0;
   RvR_fix16 offy = 0;
   RvR_fix16 offz = 0;

   //Faster movement
   if(RvR_key_down(RVR_KEY_LSHIFT))
      speed = 4;

   //Forward/Backward movement
   if(RvR_key_down(RVR_KEY_UP))
   {
      offx += speed * dirx;
      offy += speed * diry;
   }
   else if(RvR_key_down(RVR_KEY_DOWN))
   {
      offx -= speed * dirx;
      offy -= speed * diry;
   }

   //Up/Down movement or shearing
   if(RvR_key_down(RVR_KEY_A))
   {
      if(RvR_key_down(RVR_KEY_LALT))
         camera.shear += CAMERA_SHEAR_STEP_FRAME;
      else
         offz = speed * 64 * 64;
   }
   else if(RvR_key_down(RVR_KEY_Z))
   {
      if(RvR_key_down(RVR_KEY_LALT))
         camera.shear -= CAMERA_SHEAR_STEP_FRAME;
      else
         offz = -speed * 64 * 64;
   }

   camera.shear = RvR_max(-CAMERA_SHEAR_MAX_PIXELS, RvR_min(CAMERA_SHEAR_MAX_PIXELS, camera.shear));

   //Rotation or left/right movement
   if(RvR_key_down(RVR_KEY_LEFT))
   {
      if(RvR_key_down(RVR_KEY_RCTRL))
      {
         offx += speed * diry;
         offy -= speed * dirx;
      }
      else
      {
         camera.dir -= 64 * 8;
      }
   }
   else if(RvR_key_down(RVR_KEY_RIGHT))
   {
      if(RvR_key_down(RVR_KEY_RCTRL))
      {
         offx -= speed * diry;
         offy += speed * dirx;
      }
      else
      {
         camera.dir += 64 * 8;
      }
   }
   camera.dir &= 65535;

   //Collision
   RvR_fix16 floor_height = 0;
   RvR_fix16 ceiling_height = 0;
   move_with_collision(offx, offy, offz, 1, 1, &floor_height, &ceiling_height);
}

static void move_with_collision(RvR_fix16 offx, RvR_fix16 offy, RvR_fix16 offz, int8_t compute_height, int8_t compute_plane, RvR_fix16 *floor_height, RvR_fix16 *ceiling_height)
{
   int8_t moves_in_plane = offx!=0||offy!=0;
   int collide_x = 0;
   int collide_y = 0;

   //Check x collision
   for(int y = -((CAMERA_COLL_RADIUS + 65535) / 65536); y<(CAMERA_COLL_RADIUS + 65535) / 65536; y++)
   {

      /*for(int x = -((CAMERA_COLL_RADIUS+65535)/65536);x<(CAMERA_COLL_RADIUS+65535)/65536;x++)
      {
         if(compute_height)
         {
            RvR_fix16 height = RvR_ray_map_floor_height_at(map,camera.x/65536+x,camera.y/65536+y);
         }
      }*/
   }

   if(compute_plane)
   {
      int16_t x_square_new, y_square_new;

      RvR_fix16 cornerx;
      RvR_fix16 cornery;
      RvR_fix16 corner_newx;
      RvR_fix16 corner_newy;
      //RvR_fix22_vec2 corner; // BBox corner in the movement direction
      //RvR_fix22_vec2 corner_new;

      int16_t x_dir = offx>0?1:-1;
      int16_t y_dir = offy>0?1:-1;

      cornerx = camera.x + x_dir * CAMERA_COLL_RADIUS;
      cornery = camera.y + y_dir * CAMERA_COLL_RADIUS;

      //int16_t x_square = RvR_div_round_down(corner.x, 1024);
      //int16_t y_square = RvR_div_round_down(corner.y, 1024);
      //TODO: why round down?
      int16_t x_square = (cornerx / 65536);
      int16_t y_square = (cornery / 65536);

      corner_newx = cornerx + offx;
      corner_newy = cornery + offy;

      //x_square_new = RvR_div_round_down(corner_new.x, 1024);
      //y_square_new = RvR_div_round_down(corner_new.y, 1024);
      x_square_new = (corner_newx / 65536);
      y_square_new = (corner_newy / 65536);

      //TODO
      RvR_fix16 bottom_limit = -128 * 65536;
      RvR_fix16 top_limit = 128 * 65536;

      RvR_fix16 curr_ceil_height = 128 * 65536;

      if(compute_height)
      {
         bottom_limit = camera.z - CAMERA_COLL_HEIGHT_BELOW + CAMERA_COLL_STEP_HEIGHT;

         top_limit = camera.z + CAMERA_COLL_HEIGHT_ABOVE;

         curr_ceil_height = RvR_ray_map_ceiling_height_at(map, x_square, y_square);
      }

      // checks a single square for collision against the camera
#define collCheck(dir, s1, s2) \
   if(compute_height) \
   { \
      RvR_fix16 height = RvR_ray_map_floor_height_at(map, s1, s2); \
      if(height>bottom_limit|| \
         curr_ceil_height - height< \
         CAMERA_COLL_HEIGHT_BELOW + CAMERA_COLL_HEIGHT_ABOVE) \
      dir ## _collides = 1; \
      else \
      { \
         RvR_fix16 height2 = RvR_ray_map_ceiling_height_at(map, s1, s2); \
         if((height2<top_limit)||((height2 - height)< \
                                  (CAMERA_COLL_HEIGHT_ABOVE + CAMERA_COLL_HEIGHT_BELOW))) \
         dir ## _collides = 1; \
      } \
   } \
   else \
   dir ## _collides = RvR_ray_map_floor_height_at(map, s1, s2)>CAMERA_COLL_STEP_HEIGHT;

      // check collision against non-diagonal square
#define collCheckOrtho(dir, dir2, s1, s2, x) \
   if (dir ## _square_new != dir ## _square) \
   { \
      collCheck(dir, s1, s2) \
   } \
   if(!dir ## _collides) \
   {    /* now also check for coll on the neighbouring square */ \
      int16_t dir2 ## _square2 = (corner ## dir2 - dir2 ## _dir * CAMERA_COLL_RADIUS * 2) / 65536; \
      if(dir2 ## _square2!=dir2 ## _square) \
      { \
         if(x) \
         collCheck(dir, dir ## _square_new, dir2 ## _square2) \
         else \
         collCheck(dir, dir2 ## _square2, dir ## _square_new) \
      } \
   }
      //int16_t dir2##_square2 = RvR_div_round_down(corner.dir2-dir2##_dir *
      //int16_t dir2 ## _square2 = (corner ## dir2 - dir2## _dir * CAMERA_COLL_RADIUS*2)/65536; RvR_div_round_down(corner ## dir2 - dir2 ## _dir *
      //CAMERA_COLL_RADIUS * 2, 1024);

      int8_t x_collides = 0;
      collCheckOrtho(x, y, x_square_new, y_square, 1)

      int8_t y_collides = 0;
      collCheckOrtho(y, x, x_square, y_square_new, 0)

      if(x_collides||y_collides)
      {
         if(moves_in_plane)
         {
            #define collHandle(dir) \
   if (dir ## _collides) \
   corner_new ## dir = (dir ## _square)*65536 + \
                       65536 / 2 + dir ## _dir * (65536 / 2) - \
                       dir ## _dir; \

            collHandle(x)
            collHandle(y)

            #undef collHandle
         }
         else
         {
            /* Player collides without moving in the plane; this can happen e.g. on
            elevators due to vertical only movement. This code can get executed
            when force == 1. */

            RvR_fix16 square_posx;
            RvR_fix16 square_posy;
            RvR_fix16 new_posx;
            RvR_fix16 new_posy;
            //RvR_fix22_vec2 square_pos;
            //RvR_fix22_vec2 new_pos;

            square_posx = x_square * 65536;
            square_posy = y_square * 65536;

            new_posx = RvR_max(square_posx + CAMERA_COLL_RADIUS + 1, RvR_min(square_posx + 65536 - CAMERA_COLL_RADIUS - 1, camera.x));
            new_posy = RvR_max(square_posy + CAMERA_COLL_RADIUS + 1, RvR_min(square_posy + 65536 - CAMERA_COLL_RADIUS - 1, camera.y));

            corner_newx = cornerx + (new_posx - camera.x);
            corner_newy = cornery + (new_posy - camera.y);
         }
      }
      else
      {
         /* If no non-diagonal collision is detected, a diagonal/corner collision
         can still happen, check it here. */

         if(x_square!=x_square_new&&y_square!=y_square_new)
         {
            int8_t xy_collides = 0;
            collCheck(xy, x_square_new, y_square_new)

            if (xy_collides)
            {
               corner_newx = cornerx;
               corner_newy = cornery;
            }
         }
      }

#undef collCheck

      camera.x = corner_newx - x_dir * CAMERA_COLL_RADIUS;
      camera.y = corner_newy - y_dir * CAMERA_COLL_RADIUS;
   }

   if(compute_height&&(moves_in_plane||(offz!=0)||1))
   {
      camera.z += offz;

      //int16_t x_square1 = RvR_div_round_down(camera.pos.x-CAMERA_COLL_RADIUS,1024);
      int16_t x_square1 = (camera.x - CAMERA_COLL_RADIUS) / 65536;
      int16_t x_square2 = (camera.x + CAMERA_COLL_RADIUS) / 65536;
      int16_t y_square1 = (camera.y - CAMERA_COLL_RADIUS) / 65536;
      int16_t y_square2 = (camera.y + CAMERA_COLL_RADIUS) / 65536;
      //printf("(%d %d) (%d %d)\n",x_square1,y_square1,x_square2,y_square2);

      //int16_t x_square2 = RvR_div_round_down(camera.pos.x + CAMERA_COLL_RADIUS, 1024);

      //int16_t y_square1 = RvR_div_round_down(camera.pos.y - CAMERA_COLL_RADIUS, 1024);

      //int16_t y_square2 = RvR_div_round_down(camera.pos.y + CAMERA_COLL_RADIUS, 1024);

      RvR_fix16 bottom_limit = RvR_ray_map_floor_height_at(map, x_square1, y_square1);
      RvR_fix16 top_limit = RvR_ray_map_ceiling_height_at(map, x_square1, y_square1);

      RvR_fix16 height;

#define checkSquares(s1, s2) \
   { \
      height = RvR_ray_map_floor_height_at(map, x_square ## s1, y_square ## s2); \
      bottom_limit = RvR_max(bottom_limit, height); \
      height = RvR_ray_map_ceiling_height_at(map, x_square ## s1, y_square ## s2); \
      top_limit = RvR_min(top_limit, height); \
   }

      if(x_square2!=x_square1)
         checkSquares(2, 1)

         if(y_square2!=y_square1)
            checkSquares(1, 2)

            if(x_square2!=x_square1&&y_square2!=y_square1)
               checkSquares(2, 2)

               if(floor_height!=NULL)
                  *floor_height = bottom_limit;

      if(ceiling_height!=NULL)
         *ceiling_height = top_limit;

      camera.z = RvR_clamp(camera.z, bottom_limit + CAMERA_COLL_HEIGHT_BELOW, top_limit - CAMERA_COLL_HEIGHT_ABOVE);

#undef checkSquares
   }
}

static void undo_write(uint16_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos + 1);
   undo_len += (undo_len<UNDO_BUFFER_SIZE - 2);
   redo_len -= (redo_len>0);
   undo_entry_len++;
}

static void redo_write(uint16_t val)
{
   int pos = undo_pos;
   undo_buffer[pos] = val;
   undo_pos = WRAP(pos - 1);
   redo_len += (redo_len<UNDO_BUFFER_SIZE - 2);
   undo_len -= (undo_len>0);
}

static void undo_begin(Ed_action action)
{
   redo_len = 0;
   undo_write(UNDO_RECORD);
   undo_write(action);
   undo_entry_len = 0;
}

static void undo_end()
{
   int pos = WRAP(undo_pos - 1);
   //int pos = (undo_pos-1)&(UNDO_BUFFER_SIZE-1);
   if(undo_buffer[pos]==UNDO_RECORD)
   {
      //empty
      undo_pos = pos;
      undo_len -= 1;
   }
   else
   {
      uint16_t hi = (undo_entry_len >> 16) & UINT16_MAX;
      uint16_t lo = undo_entry_len & UINT16_MAX;
      undo_write(hi);
      undo_write(lo);
   }
   undo_buffer[undo_pos] = JUNK_RECORD;
}

static void undo_record_height(int16_t x, int16_t y, RvR_fix16 height)
{
   undo_write(x);
   undo_write(y);
   undo_write((height >> 16) & UINT16_MAX);
   undo_write(height & UINT16_MAX);
}

static void redo_record_height(int16_t x, int16_t y, RvR_fix16 height)
{
   redo_write(x);
   redo_write(y);
   redo_write((height >> 16) & UINT16_MAX);
   redo_write(height & UINT16_MAX);
}

static void undo_record_tex(int16_t x, int16_t y, uint16_t tex)
{
   undo_write(x);
   undo_write(y);
   undo_write(tex);
}

static void redo_record_tex(int16_t x, int16_t y, uint16_t tex)
{
   redo_write(x);
   redo_write(y);
   redo_write(tex);
}

static int undo_find_end(uint32_t *len)
{
   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return -1;

   int pos = WRAP(undo_pos - 1);
   *len = undo_buffer[pos]; pos = WRAP(pos - 1);
   *len += undo_buffer[pos] << 16;

   return WRAP(pos - *len - 1);
}

static int redo_find_end(uint32_t *len)
{
   if(undo_buffer[undo_pos]!=JUNK_RECORD)
      return -1;
   if(redo_len<=0)
      return -1;

   int pos = WRAP(undo_pos + 1);
   *len = undo_buffer[pos]; pos = WRAP(pos + 1);
   *len += undo_buffer[pos] << 16;

   return WRAP(pos + *len + 1);
}

static void undo_floor_height(int pos, int endpos)
{
   while(pos!=endpos)
   {
      RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos - 1);
      height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos - 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos - 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos - 1);
      RvR_fix16 old_height = RvR_ray_map_floor_height_at(map, x, y);

      redo_record_height(x, y, old_height);
      RvR_ray_map_floor_height_set(map, x, y, height);
   }
}

static void redo_floor_height(int pos, int endpos)
{
   while(pos!=endpos)
   {
      RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos + 1);
      height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos + 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos + 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos + 1);
      RvR_fix16 old_height = RvR_ray_map_floor_height_at(map, x, y);

      undo_record_height(x, y, old_height);
      RvR_ray_map_floor_height_set(map, x, y, height);
   }
}

static void undo_ceiling_height(int pos, int endpos)
{
   while(pos!=endpos)
   {
      RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos - 1);
      height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos - 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos - 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos - 1);
      RvR_fix16 old_height = RvR_ray_map_ceiling_height_at(map, x, y);

      redo_record_height(x, y, old_height);
      RvR_ray_map_ceiling_height_set(map, x, y, height);
   }
}

static void redo_ceiling_height(int pos, int endpos)
{
   while(pos!=endpos)
   {
      RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos + 1);
      height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos + 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos + 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos + 1);
      RvR_fix16 old_height = RvR_ray_map_ceiling_height_at(map, x, y);

      undo_record_height(x, y, old_height);
      RvR_ray_map_ceiling_height_set(map, x, y, height);
   }
}

static void undo_flood_floor_height(int pos, int endpos)
{
   RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos - 1);
   height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos - 1);
   RvR_fix16 old_height = INT32_MIN;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos];        pos = WRAP(pos - 1);
      x = undo_buffer[pos];        pos = WRAP(pos - 1);

      if(old_height==INT32_MIN)
         old_height = RvR_ray_map_floor_height_at(map, x, y);
      RvR_ray_map_floor_height_set(map, x, y, height);

      redo_write(x);
      redo_write(y);
   }

   redo_write((old_height >> 16) & UINT16_MAX);
   redo_write(old_height & UINT16_MAX);
}

static void redo_flood_floor_height(int pos, int endpos)
{
   RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos + 1);
   height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos + 1);
   RvR_fix16 old_height = INT32_MIN;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos];        pos = WRAP(pos + 1);
      x = undo_buffer[pos];        pos = WRAP(pos + 1);

      if(old_height==INT32_MIN)
         old_height = RvR_ray_map_floor_height_at(map, x, y);
      RvR_ray_map_floor_height_set(map, x, y, height);

      undo_write(x);
      undo_write(y);
   }

   undo_write((old_height >> 16) & UINT16_MAX);
   undo_write(old_height & UINT16_MAX);
}

static void undo_flood_ceiling_height(int pos, int endpos)
{
   RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos - 1);
   height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos - 1);
   RvR_fix16 old_height = INT32_MIN;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos - 1);
      x = undo_buffer[pos]; pos = WRAP(pos - 1);

      if(old_height==INT32_MIN)
         old_height = RvR_ray_map_ceiling_height_at(map, x, y);
      RvR_ray_map_ceiling_height_set(map, x, y, height);

      redo_write(x);
      redo_write(y);
   }

   redo_write((old_height >> 16) & UINT16_MAX);
   redo_write(old_height & UINT16_MAX);
}

static void redo_flood_ceiling_height(int pos, int endpos)
{
   RvR_fix16 height = undo_buffer[pos]; pos = WRAP(pos + 1);
   height += ((uint32_t)undo_buffer[pos]) << 16;        pos = WRAP(pos + 1);
   RvR_fix16 old_height = INT32_MIN;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos + 1);
      x = undo_buffer[pos]; pos = WRAP(pos + 1);

      if(old_height==INT32_MIN)
         old_height = RvR_ray_map_ceiling_height_at(map, x, y);
      RvR_ray_map_ceiling_height_set(map, x, y, height);

      undo_write(x);
      undo_write(y);
   }

   undo_write((old_height >> 16) & UINT16_MAX);
   undo_write(old_height & UINT16_MAX);
}

static void undo_floor_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos - 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos - 1);
      uint16_t old_tex = RvR_ray_map_floor_tex_at(map, x, y);

      redo_record_tex(x, y, old_tex);
      RvR_ray_map_floor_tex_set(map, x, y, tex);
   }
}

static void redo_floor_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
      int16_t y = undo_buffer[pos];    pos = WRAP(pos + 1);
      int16_t x = undo_buffer[pos];    pos = WRAP(pos + 1);
      uint16_t old_tex = RvR_ray_map_floor_tex_at(map, x, y);

      undo_record_tex(x, y, old_tex);
      RvR_ray_map_floor_tex_set(map, x, y, tex);
   }
}

static void undo_ceiling_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos - 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos - 1);
      uint16_t old_tex = RvR_ray_map_ceil_tex_at(map, x, y);

      redo_record_tex(x, y, old_tex);
      RvR_ray_map_ceil_tex_set(map, x, y, tex);
   }
}

static void redo_ceiling_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
      int16_t y = undo_buffer[pos];    pos = WRAP(pos + 1);
      int16_t x = undo_buffer[pos];    pos = WRAP(pos + 1);
      uint16_t old_tex = RvR_ray_map_ceil_tex_at(map, x, y);

      undo_record_tex(x, y, old_tex);
      RvR_ray_map_ceil_tex_set(map, x, y, tex);
   }
}

static void undo_floor_wall_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos - 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos - 1);
      uint16_t old_tex = RvR_ray_map_wall_ftex_at(map, x, y);

      redo_record_tex(x, y, old_tex);
      RvR_ray_map_wall_ftex_set(map, x, y, tex);
   }
}

static void redo_floor_wall_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
      int16_t y = undo_buffer[pos];    pos = WRAP(pos + 1);
      int16_t x = undo_buffer[pos];    pos = WRAP(pos + 1);
      uint16_t old_tex = RvR_ray_map_wall_ftex_at(map, x, y);

      undo_record_tex(x, y, old_tex);
      RvR_ray_map_wall_ftex_set(map, x, y, tex);
   }
}

static void undo_ceiling_wall_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
      int16_t y = undo_buffer[pos];        pos = WRAP(pos - 1);
      int16_t x = undo_buffer[pos];        pos = WRAP(pos - 1);
      uint16_t old_tex = RvR_ray_map_wall_ctex_at(map, x, y);

      redo_record_tex(x, y, old_tex);
      RvR_ray_map_wall_ctex_set(map, x, y, tex);
   }
}

static void redo_ceiling_wall_tex(int pos, int endpos)
{
   while(pos!=endpos)
   {
      uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
      int16_t y = undo_buffer[pos];    pos = WRAP(pos + 1);
      int16_t x = undo_buffer[pos];    pos = WRAP(pos + 1);
      uint16_t old_tex = RvR_ray_map_wall_ctex_at(map, x, y);

      undo_record_tex(x, y, old_tex);
      RvR_ray_map_wall_ctex_set(map, x, y, tex);
   }
}

static void undo_flood_floor_tex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos - 1);
      x = undo_buffer[pos]; pos = WRAP(pos - 1);

      old_tex = RvR_ray_map_floor_tex_at(map, x, y);
      RvR_ray_map_floor_tex_set(map, x, y, tex);

      redo_write(x);
      redo_write(y);
   }

   redo_write(old_tex);
}

static void redo_flood_floor_tex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos + 1);
      x = undo_buffer[pos]; pos = WRAP(pos + 1);

      old_tex = RvR_ray_map_floor_tex_at(map, x, y);
      RvR_ray_map_floor_tex_set(map, x, y, tex);

      undo_write(x);
      undo_write(y);
   }

   undo_write(old_tex);
}

static void undo_flood_ceiling_tex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos - 1);
      x = undo_buffer[pos]; pos = WRAP(pos - 1);

      old_tex = RvR_ray_map_ceil_tex_at(map, x, y);
      RvR_ray_map_ceil_tex_set(map, x, y, tex);

      redo_write(x);
      redo_write(y);
   }

   redo_write(old_tex);
}

static void redo_flood_ceiling_tex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos + 1);
      x = undo_buffer[pos]; pos = WRAP(pos + 1);

      old_tex = RvR_ray_map_ceil_tex_at(map, x, y);
      RvR_ray_map_ceil_tex_set(map, x, y, tex);

      undo_write(x);
      undo_write(y);
   }

   undo_write(old_tex);
}

static void undo_flood_wall_ftex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos - 1);
      x = undo_buffer[pos]; pos = WRAP(pos - 1);

      old_tex = RvR_ray_map_wall_ftex_at(map, x, y);
      RvR_ray_map_wall_ftex_set(map, x, y, tex);

      redo_write(x);
      redo_write(y);
   }

   redo_write(old_tex);
}

static void redo_flood_wall_ftex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos + 1);
      x = undo_buffer[pos]; pos = WRAP(pos + 1);

      old_tex = RvR_ray_map_wall_ftex_at(map, x, y);
      RvR_ray_map_wall_ftex_set(map, x, y, tex);

      undo_write(x);
      undo_write(y);
   }

   undo_write(old_tex);
}

static void undo_flood_wall_ctex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos - 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos - 1);
      x = undo_buffer[pos]; pos = WRAP(pos - 1);

      old_tex = RvR_ray_map_wall_ctex_at(map, x, y);
      RvR_ray_map_wall_ctex_set(map, x, y, tex);

      redo_write(x);
      redo_write(y);
   }

   redo_write(old_tex);
}

static void redo_flood_wall_ctex(int pos, int endpos)
{
   uint16_t tex = undo_buffer[pos]; pos = WRAP(pos + 1);
   uint16_t old_tex = 0;
   int16_t x = 0;
   int16_t y = 0;

   while(pos!=endpos)
   {
      y = undo_buffer[pos]; pos = WRAP(pos + 1);
      x = undo_buffer[pos]; pos = WRAP(pos + 1);

      old_tex = RvR_ray_map_wall_ctex_at(map, x, y);
      RvR_ray_map_wall_ctex_set(map, x, y, tex);

      undo_write(x);
      undo_write(y);
   }

   undo_write(old_tex);
}

void editor_ed_floor(int16_t x, int16_t y, int fac)
{
   undo_begin(ED_FLOOR_HEIGHT);
   undo_record_height(x, y, RvR_ray_map_floor_height_at(map, x, y));
   RvR_ray_map_floor_height_set(map, x, y, RvR_ray_map_floor_height_at(map, x, y) + 128 * 64 * fac);
   undo_end();
}

void editor_ed_ceiling(int16_t x, int16_t y, int fac)
{
   undo_begin(ED_CEILING_HEIGHT);
   undo_record_height(x, y, RvR_ray_map_ceiling_height_at(map, x, y));
   RvR_ray_map_ceiling_height_set(map, x, y, RvR_ray_map_ceiling_height_at(map, x, y) + 128 * 64 * fac);
   undo_end();
}

void editor_ed_flood_floor(int16_t x, int16_t y, int fac)
{
   if(fac==0)
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   undo_begin(ED_FLOOD_FLOOR_HEIGHT);

   RvR_fix16 height = RvR_ray_map_floor_height_at(map, x, y);
   uint16_t ftex = RvR_ray_map_floor_tex_at(map, x, y);

   flood_floor_height(x, y, ftex, height, fac);

   undo_write((height >> 16) & UINT16_MAX);
   undo_write(height & UINT16_MAX);

   undo_end();
}

void editor_ed_flood_ceiling(int16_t x, int16_t y, int fac)
{
   if(fac==0)
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   undo_begin(ED_FLOOD_CEILING_HEIGHT);

   RvR_fix16 height = RvR_ray_map_ceiling_height_at(map, x, y);
   uint16_t ctex = RvR_ray_map_ceil_tex_at(map, x, y);

   flood_ceiling_height(x, y, ctex, height, fac);

   undo_write((height >> 16) & UINT16_MAX);
   undo_write(height & UINT16_MAX);

   undo_end();
}

static void flood_floor_height(int16_t x, int16_t y, uint16_t ftex, RvR_fix16 height, int fac)
{
   if(RvR_ray_map_inbounds(map, x, y)&&RvR_ray_map_floor_height_at(map, x, y)==height&&RvR_ray_map_floor_tex_at(map, x, y)==ftex)
   {
      RvR_ray_map_floor_height_set(map, x, y, height + fac * 128 * 64);
      undo_write(x);
      undo_write(y);

      flood_floor_height(x - 1, y, ftex, height, fac);
      flood_floor_height(x + 1, y, ftex, height, fac);
      flood_floor_height(x, y - 1, ftex, height, fac);
      flood_floor_height(x, y + 1, ftex, height, fac);
   }
}

static void flood_ceiling_height(int16_t x, int16_t y, uint16_t ctex, RvR_fix16 height, int fac)
{
   if(RvR_ray_map_inbounds(map, x, y)&&RvR_ray_map_ceiling_height_at(map, x, y)==height&&RvR_ray_map_ceil_tex_at(map, x, y)==ctex)
   {
      RvR_ray_map_ceiling_height_set(map, x, y, height + fac * 128 * 64);
      undo_write(x);
      undo_write(y);

      flood_ceiling_height(x - 1, y, ctex, height, fac);
      flood_ceiling_height(x + 1, y, ctex, height, fac);
      flood_ceiling_height(x, y - 1, ctex, height, fac);
      flood_ceiling_height(x, y + 1, ctex, height, fac);
   }
}

static void flood_floor_tex(int16_t x, int16_t y, uint16_t ftex, uint16_t tex, RvR_fix16 height)
{
   if(RvR_ray_map_inbounds(map, x, y)&&RvR_ray_map_floor_height_at(map, x, y)==height&&RvR_ray_map_floor_tex_at(map, x, y)==tex)
   {
      RvR_ray_map_floor_tex_set(map, x, y, ftex);
      undo_write(x);
      undo_write(y);

      flood_floor_tex(x - 1, y, ftex, tex, height);
      flood_floor_tex(x + 1, y, ftex, tex, height);
      flood_floor_tex(x, y - 1, ftex, tex, height);
      flood_floor_tex(x, y + 1, ftex, tex, height);
   }
}

static void flood_ceiling_tex(int16_t x, int16_t y, uint16_t ctex, uint16_t tex, RvR_fix16 height)
{
   if(RvR_ray_map_inbounds(map, x, y)&&RvR_ray_map_ceiling_height_at(map, x, y)==height&&RvR_ray_map_ceil_tex_at(map, x, y)==tex)
   {
      RvR_ray_map_ceil_tex_set(map, x, y, ctex);
      undo_write(x);
      undo_write(y);

      flood_ceiling_tex(x - 1, y, ctex, tex, height);
      flood_ceiling_tex(x + 1, y, ctex, tex, height);
      flood_ceiling_tex(x, y - 1, ctex, tex, height);
      flood_ceiling_tex(x, y + 1, ctex, tex, height);
   }
}

static void flood_wall_ftex(int16_t x, int16_t y, uint16_t ftex, uint16_t tex, RvR_fix16 height)
{
   if(RvR_ray_map_inbounds(map, x, y)&&RvR_ray_map_floor_height_at(map, x, y)==height&&RvR_ray_map_wall_ftex_at(map, x, y)==tex)
   {
      RvR_ray_map_wall_ftex_set(map, x, y, ftex);
      undo_write(x);
      undo_write(y);

      flood_wall_ftex(x - 1, y, ftex, tex, height);
      flood_wall_ftex(x + 1, y, ftex, tex, height);
      flood_wall_ftex(x, y - 1, ftex, tex, height);
      flood_wall_ftex(x, y + 1, ftex, tex, height);
   }
}

static void flood_wall_ctex(int16_t x, int16_t y, uint16_t ctex, uint16_t tex, RvR_fix16 height)
{
   if(RvR_ray_map_inbounds(map, x, y)&&RvR_ray_map_ceiling_height_at(map, x, y)==height&&RvR_ray_map_wall_ctex_at(map, x, y)==tex)
   {
      RvR_ray_map_wall_ctex_set(map, x, y, ctex);
      undo_write(x);
      undo_write(y);

      flood_wall_ctex(x - 1, y, ctex, tex, height);
      flood_wall_ctex(x + 1, y, ctex, tex, height);
      flood_wall_ctex(x, y - 1, ctex, tex, height);
      flood_wall_ctex(x, y + 1, ctex, tex, height);
   }
}

void editor_ed_floor_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   if(RvR_ray_map_floor_tex_at(map, x, y)==tex)
      return;

   undo_begin(ED_FLOOR_TEXTURE);
   undo_record_tex(x, y, RvR_ray_map_floor_tex_at(map, x, y));
   RvR_ray_map_floor_tex_set(map, x, y, tex);
   undo_end();
}

void editor_ed_ceiling_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   if(RvR_ray_map_ceil_tex_at(map, x, y)==tex)
      return;

   undo_begin(ED_CEILING_TEXTURE);
   undo_record_tex(x, y, RvR_ray_map_ceil_tex_at(map, x, y));
   RvR_ray_map_ceil_tex_set(map, x, y, tex);
   undo_end();
}

void editor_ed_floor_wall_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   if(RvR_ray_map_wall_ftex_at(map, x, y)==tex)
      return;

   undo_begin(ED_FLOOR_WALL_TEXTURE);
   undo_record_tex(x, y, RvR_ray_map_wall_ftex_at(map, x, y));
   RvR_ray_map_wall_ftex_set(map, x, y, tex);
   undo_end();
}

void editor_ed_ceiling_wall_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   if(RvR_ray_map_wall_ctex_at(map, x, y)==tex)
      return;

   undo_begin(ED_CEILING_WALL_TEXTURE);
   undo_record_tex(x, y, RvR_ray_map_wall_ctex_at(map, x, y));
   RvR_ray_map_wall_ctex_set(map, x, y, tex);
   undo_end();
}

void editor_ed_flood_floor_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   RvR_fix16 height = RvR_ray_map_floor_height_at(map, x, y);
   uint16_t ftex = RvR_ray_map_floor_tex_at(map, x, y);

   if(ftex==tex)
      return;

   undo_begin(ED_FLOOD_FLOOR_TEXTURE);
   flood_floor_tex(x, y, tex, ftex, height);

   undo_write(ftex);
   undo_end();
}

void editor_ed_flood_ceiling_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   RvR_fix16 height = RvR_ray_map_ceiling_height_at(map, x, y);
   uint16_t ctex = RvR_ray_map_ceil_tex_at(map, x, y);

   if(ctex==tex)
      return;

   undo_begin(ED_FLOOD_CEILING_TEXTURE);
   flood_ceiling_tex(x, y, tex, ctex, height);

   undo_write(ctex);
   undo_end();
}

void editor_ed_flood_floor_wall_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   RvR_fix16 height = RvR_ray_map_floor_height_at(map, x, y);
   uint16_t ftex = RvR_ray_map_wall_ftex_at(map, x, y);

   if(ftex==tex)
      return;

   undo_begin(ED_FLOOD_FLOOR_WALL_TEXTURE);
   flood_wall_ftex(x, y, tex, ftex, height);

   undo_write(ftex);
   undo_end();
}

void editor_ed_flood_ceiling_wall_tex(int16_t x, int16_t y, uint16_t tex)
{
   if(!texture_valid(tex))
      return;

   if(!RvR_ray_map_inbounds(map, x, y))
      return;

   RvR_fix16 height = RvR_ray_map_ceiling_height_at(map, x, y);
   uint16_t ctex = RvR_ray_map_wall_ctex_at(map, x, y);

   if(ctex==tex)
      return;

   undo_begin(ED_FLOOD_CEILING_WALL_TEXTURE);
   flood_wall_ctex(x, y, tex, ctex, height);

   undo_write(ctex);
   undo_end();
}
//-------------------------------------

#undef CAMERA_SHEAR_MAX_PIXELS
#undef CAMERA_SHEAR_STEP_FRAME
