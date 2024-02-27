/*
RvnicRaven-portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PORT_RENDER_H_

#define _RVR_PORT_RENDER_H_

typedef struct
{
   uint32_t flags;
   uint16_t sector;
   uint16_t texture;
   void *ref;

   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
   RvR_fix22 dir;

   RvR_fix22 z_min;
   RvR_fix22 z_max;

   union
   {
      struct
      {
         //Camera space coordinates
         //wy is depth
         RvR_fix22 wx;
         RvR_fix22 wy;
      }bill;
      struct
      {
         RvR_fix22 x0;
         RvR_fix22 xw0;
         RvR_fix22 z0;
         RvR_fix22 zw0;
         RvR_fix22 x1;
         RvR_fix22 xw1;
         RvR_fix22 z1;
         RvR_fix22 zw1;

         RvR_fix22 u0;
         RvR_fix22 u1;
      }wall;
      struct
      {
         RvR_fix22 x0;
         RvR_fix22 z0;
         RvR_fix22 x1;
         RvR_fix22 z1;
         RvR_fix22 x2;
         RvR_fix22 z2;
         RvR_fix22 x3;
         RvR_fix22 z3;

         //Camera space coordinates
         //wy is depth
         RvR_fix22 wx;
         RvR_fix22 wy;
      }floor;
   }as;
}port_sprite;

typedef struct
{
   RvR_port_depth_buffer_entry *floor[RVR_XRES_MAX];
   RvR_port_depth_buffer_entry *ceiling[RVR_XRES_MAX];
} RvR_port_depth_buffer;

RvR_port_depth_buffer_entry *port_depth_buffer_entry_new();
void port_depth_buffer_entry_free(RvR_port_depth_buffer_entry *ent);
void sprites_render(RvR_port_selection *select);

void port_sprite_draw_floor(const port_sprite *sp, RvR_port_selection *select);
void port_sprite_draw_wall(const port_sprite *sp, RvR_port_selection *select);
void port_sprite_draw_billboard(const RvR_port_map *map, const port_sprite *sp, RvR_port_selection *select);

extern RvR_port_depth_buffer port_depth_buffer;

extern port_sprite *port_sprites;

extern const RvR_port_cam *port_cam;
extern const RvR_port_map *port_map;

extern RvR_port_report port_report;

#endif
