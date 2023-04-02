/*
RvnicRaven - raycast drawing

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR/RvR_ray.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_ray_draw_begin()
{
   //Clear depth buffer
   for(int i = 0;i<RVR_XRES;i++)
   {
      ray_depth_buffer_entry_free(ray_depth_buffer.floor[i]);
      ray_depth_buffer_entry_free(ray_depth_buffer.ceiling[i]);

      ray_depth_buffer.floor[i] = NULL;
      ray_depth_buffer.ceiling[i] = NULL;
   }

   //Clear planes
   for(int i = 0;i<128;i++)
   {
      ray_plane_free(ray_planes[i]);
      ray_planes[i] = NULL;
   }

   //Initialize needed vars
   ray_fov_factor_x = RvR_fix22_tan(RvR_ray_get_fov()/2);
   ray_fov_factor_y = (RVR_YRES*ray_fov_factor_x*2)/RVR_XRES;

   ray_middle_row = (RVR_YRES/2)+RvR_ray_get_shear();

   ray_start_floor_height = RvR_ray_map_floor_height_at(RvR_ray_get_position().x/1024,RvR_ray_get_position().y/1024)-RvR_ray_get_position().z;
   ray_start_ceil_height = RvR_ray_map_ceiling_height_at(RvR_ray_get_position().x/1024,RvR_ray_get_position().y/1024)-RvR_ray_get_position().z;

   ray_cos = RvR_fix22_cos(RvR_ray_get_angle());
   ray_sin = RvR_fix22_sin(RvR_ray_get_angle());
   ray_cos_fov = (ray_cos*ray_fov_factor_x)/1024;
   ray_sin_fov = (ray_sin*ray_fov_factor_x)/1024;
}

void RvR_ray_draw_end()
{
}
//-------------------------------------
