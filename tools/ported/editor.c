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
//-------------------------------------

//#defines
#define CAMERA_SHEAR_MAX_PIXELS ((CAMERA_SHEAR_MAX * RvR_yres()) / 65536)
#define CAMERA_SHEAR_STEP_FRAME ((RvR_yres() * CAMERA_SHEAR_SPEED) / (RvR_fps() * 4))
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
RvR_port_cam camera;

static int editor_mode = 0;

static uint16_t *undo_buffer = NULL;
static int undo_len = 0;
static int undo_pos = 0;
static int redo_len = 0;
static uint32_t undo_entry_len = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor_init(void)
{
   undo_buffer = RvR_malloc(sizeof(*undo_buffer) * UNDO_BUFFER_SIZE, "ported undo buffer");
   memset(undo_buffer, 0, sizeof(*undo_buffer) * UNDO_BUFFER_SIZE);

   camera.fov = 1024;
   camera.shear = 0;
}

void editor_update(void)
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

void editor_draw(void)
{
   if(editor_mode==0)
      editor2d_draw();
   else
      editor3d_draw();

   //if(RvR_key_pressed(RVR_KEY_U))
      //editor_undo();

   //if(RvR_key_pressed(RVR_KEY_R))
      //editor_redo();
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
   //RvR_fix16 floor_height = 0;
   //RvR_fix16 ceiling_height = 0;
   //move_with_collision(offx, offy, offz, 1, 1, &floor_height, &ceiling_height);
}
//-------------------------------------
