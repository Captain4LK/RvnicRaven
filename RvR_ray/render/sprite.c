/*
RvnicRaven-ray

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_ray_config.h"
#include "RvR_ray_render.h"
#include "RvR/RvR_ray.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
ray_sprite *ray_sprites = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_ray_draw_sprite(const RvR_ray_cam *cam, RvR_fix16 x, RvR_fix16 y, RvR_fix16 z, RvR_fix16 dir, uint16_t sprite, uint32_t flags, void *ref)
{
   ray_sprite sp = {0};
   sp.flags = flags;
   sp.texture = sprite;
   sp.x = x;
   sp.y = y;
   sp.z = z;
   sp.dir = dir;
   sp.ref = ref;

   RvR_texture *tex = RvR_texture_get(sprite);
   RvR_fix16 sin = RvR_fix16_sin(cam->dir);
   RvR_fix16 cos = RvR_fix16_cos(cam->dir);
   RvR_fix16 fovx = RvR_fix16_tan(cam->fov / 2);
   RvR_fix16 fovy = RvR_fix16_div(RvR_yres() * fovx * 2, RvR_xres() << 16);
   RvR_fix16 sin_fov = RvR_fix16_mul(sin, fovx);
   RvR_fix16 cos_fov = RvR_fix16_mul(cos, fovx);
   RvR_fix16 middle_row = (RvR_yres() / 2) + cam->shear;

   //tagged as invisible
   if(flags & 1)
      return;

   //Wall aligned
   if(flags & 8)
   {
      //Translate sprite to world space
      RvR_fix16 dirx = RvR_fix16_cos(dir);
      RvR_fix16 diry = RvR_fix16_sin(dir);
      RvR_fix16 half_width = (tex->width * 65536) / (64 * 2);
      RvR_fix16 p0x = RvR_fix16_mul(-diry, half_width) + x;
      RvR_fix16 p0y = RvR_fix16_mul(dirx, half_width) + y;
      RvR_fix16 p1x = RvR_fix16_mul(diry, half_width) + x;
      RvR_fix16 p1y = RvR_fix16_mul(-dirx, half_width) + y;
      sp.x = x;
      sp.y = y;
      sp.z = z;
      sp.as.wall.u0 = 0;
      sp.as.wall.u1 = 65536 * tex->width - 1;

      //Translate to camera space
      RvR_fix16 x0 = p0x - cam->x;
      RvR_fix16 y0 = p0y - cam->y;
      RvR_fix16 x1 = p1x - cam->x;
      RvR_fix16 y1 = p1y - cam->y;
      RvR_fix16 tp0x = RvR_fix16_mul(-x0, sin) + RvR_fix16_mul(y0, cos);
      RvR_fix16 tp0y = RvR_fix16_mul(x0, cos_fov) + RvR_fix16_mul(y0, sin_fov);
      RvR_fix16 tp1x = RvR_fix16_mul(-x1, sin) + RvR_fix16_mul(y1, cos);
      RvR_fix16 tp1y = RvR_fix16_mul(x1, cos_fov) + RvR_fix16_mul(y1, sin_fov);

      //Behind camera
      if(tp0y<-128&&tp1y<-128)
         return;

      //Sprite not facing camera
      //--> swap p0 and p1 and toggle y-axis mirror flag
      if(RvR_fix16_mul(tp0x, tp1y) - RvR_fix16_mul(tp1x, tp0y)>0)
      {
         //One sided sprite
         if(sp.flags & 128)
            return;

         RvR_fix16 tmp = tp0x;
         tp0x = tp1x;
         tp1x = tmp;

         tmp = tp0y;
         tp0y = tp1y;
         tp1y = tmp;
         sp.flags ^= 2;
      }

      sp.as.wall.wx0 = tp0x;
      sp.as.wall.wy0 = tp0y;
      sp.as.wall.wx1 = tp1x;
      sp.as.wall.wy1 = tp1y;

      //Here we can treat everything as if we have a 90 degree
      //fov, since the rotation to camera space transforms it to
      //that
      //Check if in fov
      //Left point in fov
      if(tp0x>=-tp0y)
      {
         //Sprite completely out of sight
         if(tp0x>tp0y)
            return;

         sp.as.wall.x0 = RvR_min(RvR_xres() * 32768 + RvR_fix16_div(tp0x * (RvR_xres() / 2), tp0y), RvR_xres() * 65536);
         sp.as.wall.z0 = tp0y;
      }
      //Left point to the left of fov
      else
      {
         //Sprite completely out of sight
         if(tp1x<-tp1y)
            return;

         sp.as.wall.x0 = 0;
         RvR_fix16 dx0 = tp1x - tp0x;
         RvR_fix16 dx1 = tp0x + tp0y;
         sp.as.wall.z0 = RvR_fix16_div(RvR_fix16_mul(dx0, dx1), tp1y - tp0y + tp1x - tp0x) - tp0x;
         sp.as.wall.u0 = sp.as.wall.u0 + RvR_fix16_div(RvR_fix16_mul(-tp0x - tp0y, sp.as.wall.u1 - sp.as.wall.u0), RvR_non_zero(tp1x - tp0x + tp1y - tp0y));
      }

      //Right point in fov
      if(tp1x<=tp1y)
      {
         //sprite completely out of sight
         if(tp1x<-tp1y)
            return;

         sp.as.wall.x1 = RvR_min(RvR_xres() * 32768 + RvR_fix16_div(tp1x * (RvR_xres() / 2), tp1y), RvR_xres() * 65536);
         sp.as.wall.z1 = tp1y;
      }
      //Right point to the right of fov
      else
      {
         //sprite completely out of sight
         if(tp0x>tp0y)
            return;

         RvR_fix16 dx0 = tp1x - tp0x;
         RvR_fix16 dx1 = tp0y - tp0x;
         sp.as.wall.x1 = RvR_xres() * 65536;
         sp.as.wall.z1 = tp0x - RvR_fix16_div(RvR_fix16_mul(dx0, dx1), tp1y - tp0y - tp1x + tp0x);
         sp.as.wall.u1 = RvR_fix16_div(RvR_fix16_mul(dx1, sp.as.wall.u1), RvR_non_zero(-tp1y + tp0y + tp1x - tp0x));
      }

      //Near clip sprite
      if(sp.as.wall.z0<1024||sp.as.wall.z1<1024)
         return;

      //Far clip sprite
      if(sp.as.wall.z0>RVR_RAY_MAX_STEPS * 65536&&sp.as.wall.z1>RVR_RAY_MAX_STEPS * 65536)
         return;

      if(sp.as.wall.x0>sp.as.wall.x1)
         return;

      sp.z_min = RvR_min(sp.as.wall.z0, sp.as.wall.z1);
      sp.z_max = RvR_max(sp.as.wall.z0, sp.as.wall.z1);

      RvR_array_push(ray_sprites, sp);

      return;
   }

   //Floor alligned
   if(flags & 16)
   {
      //World space coordinates, origin at camera
      RvR_fix16 scos = RvR_fix16_cos(dir);
      RvR_fix16 ssin = RvR_fix16_sin(dir);
      RvR_fix16 half_width = (tex->width * 65536) / (64 * 2);
      RvR_fix16 half_height = (tex->height * 65536) / (64 * 2);
      RvR_fix16 x0 = RvR_fix16_mul(-half_width, -ssin) + RvR_fix16_mul(-half_height, scos) + x - cam->x;
      RvR_fix16 y0 = RvR_fix16_mul(-half_width, scos) + RvR_fix16_mul(-half_height, ssin) + y - cam->y;
      RvR_fix16 x1 = RvR_fix16_mul(+half_width, -ssin) + RvR_fix16_mul(-half_height, scos) + x - cam->x;
      RvR_fix16 y1 = RvR_fix16_mul(+half_width, scos) + RvR_fix16_mul(-half_height, ssin) + y - cam->y;
      RvR_fix16 x2 = RvR_fix16_mul(+half_width, -ssin) + RvR_fix16_mul(+half_height, scos) + x - cam->x;
      RvR_fix16 y2 = RvR_fix16_mul(+half_width, scos) + RvR_fix16_mul(+half_height, ssin) + y - cam->y;
      RvR_fix16 x3 = RvR_fix16_mul(-half_width, -ssin) + RvR_fix16_mul(+half_height, scos) + x - cam->x;
      RvR_fix16 y3 = RvR_fix16_mul(-half_width, scos) + RvR_fix16_mul(+half_height, ssin) + y - cam->y;

      //Move to camera space
      sp.as.floor.x0 = RvR_fix16_mul(-x0, sin) + RvR_fix16_mul(y0, cos);
      sp.as.floor.z0 = RvR_fix16_mul(x0, cos_fov) + RvR_fix16_mul(y0, sin_fov);
      sp.as.floor.x1 = RvR_fix16_mul(-x1, sin) + RvR_fix16_mul(y1, cos);
      sp.as.floor.z1 = RvR_fix16_mul(x1, cos_fov) + RvR_fix16_mul(y1, sin_fov);
      sp.as.floor.x2 = RvR_fix16_mul(-x2, sin) + RvR_fix16_mul(y2, cos);
      sp.as.floor.z2 = RvR_fix16_mul(x2, cos_fov) + RvR_fix16_mul(y2, sin_fov);
      sp.as.floor.x3 = RvR_fix16_mul(-x3, sin) + RvR_fix16_mul(y3, cos);
      sp.as.floor.z3 = RvR_fix16_mul(x3, cos_fov) + RvR_fix16_mul(y3, sin_fov);

      sp.as.floor.wx = RvR_fix16_mul(-(x - cam->x), sin) + RvR_fix16_mul(y - cam->y, cos);
      sp.as.floor.wy = RvR_fix16_mul(x - cam->x, cos_fov) + RvR_fix16_mul(y - cam->y, sin_fov);

      RvR_fix16 depth_min = RvR_min(sp.as.floor.z0, RvR_min(sp.as.floor.z1, RvR_min(sp.as.floor.z2, sp.as.floor.z3)));
      RvR_fix16 depth_max = RvR_max(sp.as.floor.z0, RvR_max(sp.as.floor.z1, RvR_max(sp.as.floor.z2, sp.as.floor.z3)));

      //Near clip
      if(depth_max<128)
         return;

      //Far clip
      if(depth_min>RVR_RAY_MAX_STEPS * 65536)
         return;

      sp.z_min = depth_min;
      sp.z_max = depth_max;

      RvR_array_push(ray_sprites, sp);

      return;
   }

   //Billboard
   sp.as.bill.wx = RvR_fix16_mul(-(x - cam->x), sin) + RvR_fix16_mul(y - cam->y, cos);
   sp.as.bill.wy = RvR_fix16_mul(x - cam->x, cos_fov) + RvR_fix16_mul(y - cam->y, sin_fov);
   sp.z_min = sp.z_max = sp.as.bill.wy;
   RvR_fix16 depth = RvR_fix16_mul(x - cam->x, cos) + RvR_fix16_mul(y - cam->y, sin); //Separate depth, so that the near/far clip is not dependent on fov

   //Near clip
   if(depth<8192)
      return;

   //Far clip
   if(depth>RVR_RAY_MAX_STEPS * 65536)
      return;

   //Left of screen
   if(-sp.as.bill.wx - tex->width * 512>sp.as.bill.wy)
      return;

   //Right of screen
   if(sp.as.bill.wx - tex->width * 512>sp.as.bill.wy)
      return;

   //Above screen
   if(middle_row * RvR_fix16_mul(depth, fovy)<RvR_yres() * (z - cam->z))
      return;

   //Below screen
   if((middle_row - RvR_yres()) * RvR_fix16_mul(depth, fovy)>RvR_yres() * (z - cam->z + tex->height * 1024))
      return;

   RvR_array_push(ray_sprites, sp);
}
//-------------------------------------
