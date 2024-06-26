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
#include "editor.h"
#include "editor2d.h"
#include "editor3d.h"
#include "undo.h"
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
   ED_WALL_MOVE = 0,
}Ed_action;
//-------------------------------------

//Variables
RvR_port_cam camera;

static int editor_mode = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor_init(void)
{
   undo_init();

   camera.fov = 2048;
   camera.shear = 0;
   camera.x = 1024;
   camera.y = 1024;
}

void editor_update(void)
{
   //if(RvR_key_pressed(RVR_KEY_NP_ENTER))
   //editor_mode = !editor_mode;

   if(RvR_key_down(RVR_KEY_LCTRL)&&RvR_key_pressed(RVR_KEY_S))
      map_save();

   if(editor_mode==0)
      editor2d_update();
   else
      editor3d_update();
}

void editor_draw(void)
{
   if(editor_mode==0)
      editor2d_draw();
   else
      editor3d_draw();

   if(RvR_key_pressed(RVR_KEY_U))
      undo();

   if(RvR_key_pressed(RVR_KEY_R))
      redo();
}

void camera_update()
{
   RvR_fix22 dirx = RvR_fix22_cos(camera.dir) / 8;
   RvR_fix22 diry = RvR_fix22_sin(camera.dir) / 8;
   int speed = 1;
   RvR_fix22 offx = 0;
   RvR_fix22 offy = 0;
   RvR_fix22 offz = 0;

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
         offz = speed * 64;
   }
   else if(RvR_key_down(RVR_KEY_Z))
   {
      if(RvR_key_down(RVR_KEY_LALT))
         camera.shear -= CAMERA_SHEAR_STEP_FRAME;
      else
         offz = -speed * 64;
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
         camera.dir -= 16 * 8;
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
         camera.dir += 16 * 8;
      }
   }
   camera.dir &= 65535;

   camera.x += offx;
   camera.y += offy;
   camera.z += offz;

   camera.sector = RvR_port_sector_update(map, camera.sector, camera.x, camera.y);
   //camera.z = RvR_max(map->sectors[camera.sector].floor,RvR_min(map->sectors[camera.sector].ceiling,camera.z));
   //Collision
   //RvR_fix16 floor_height = 0;
   //RvR_fix16 ceiling_height = 0;
   //move_with_collision(offx, offy, offz, 1, 1, &floor_height, &ceiling_height);
}

void editor_set_3d(void)
{
   editor_mode = 1;
}

void editor_set_2d(void)
{
   editor_mode = 0;
}
//-------------------------------------
