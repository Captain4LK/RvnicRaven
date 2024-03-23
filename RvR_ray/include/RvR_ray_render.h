/*
RvnicRaven-ray

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_RAY_RENDER_H_

#define _RVR_RAY_RENDER_H_

typedef struct
{
   uint32_t flags;
   uint16_t texture;
   void *ref;

   RvR_fix16 x;
   RvR_fix16 y;
   RvR_fix16 z;
   RvR_fix16 dir;

   RvR_fix16 z_min;
   RvR_fix16 z_max;

   union
   {
      struct
      {
         //Camera space coordinates
         //wy is depth
         RvR_fix16 wx;
         RvR_fix16 wy;
      }bill;
      struct
      {
         RvR_fix16 u0;
         RvR_fix16 u1;

         RvR_fix16 x0;
         RvR_fix16 z0;
         RvR_fix16 x1;
         RvR_fix16 z1;

         //Camera space coordinates
         //wy is depth
         RvR_fix16 wx0;
         RvR_fix16 wy0;
         RvR_fix16 wx1;
         RvR_fix16 wy1;
      }wall;
      struct
      {
         RvR_fix16 x0;
         RvR_fix16 z0;
         RvR_fix16 x1;
         RvR_fix16 z1;
         RvR_fix16 x2;
         RvR_fix16 z2;
         RvR_fix16 x3;
         RvR_fix16 z3;

         //Camera space coordinates
         //wy is depth
         RvR_fix16 wx;
         RvR_fix16 wy;
      }floor;
   }as;
}ray_sprite;


typedef struct
{
   RvR_ray_depth_buffer_entry *floor[RVR_XRES_MAX];
   RvR_ray_depth_buffer_entry *ceiling[RVR_XRES_MAX];
} RvR_ray_depth_buffer;

extern RvR_ray_depth_buffer ray_depth_buffer;

extern ray_sprite *ray_sprites;
extern const RvR_ray_cam *ray_cam;
extern const RvR_ray_map *ray_map;
extern RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_pool;

void sprites_render(RvR_ray_selection *select);
void ray_sprite_draw_billboard(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp, RvR_ray_selection *select);
void ray_sprite_draw_floor(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp, RvR_ray_selection *select);
void ray_sprite_draw_wall(const RvR_ray_cam *cam, const RvR_ray_map *map, const ray_sprite *sp, RvR_ray_selection *select);

#endif
