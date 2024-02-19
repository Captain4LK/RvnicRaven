/*
RvnicRaven-portal

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR_port_config.h"
#include "RvR/RvR_portal.h"
#include "RvR_port_render.h"
#include "RvR_port_plane.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static RvR_port_depth_buffer_entry *port_depth_buffer_entry_pool = NULL;

const RvR_port_cam *port_cam = NULL;
const RvR_port_map *port_map = NULL;

port_sprite *port_sprites = NULL;

RvR_port_report port_report = {0};
RvR_port_depth_buffer port_depth_buffer = {0};
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_port_draw_begin(const RvR_port_map *map, const RvR_port_cam *cam)
{
   //Clear depth buffer
   for(int i = 0; i<RvR_xres(); i++)
   {
      port_depth_buffer_entry_free(port_depth_buffer.floor[i]);
      port_depth_buffer_entry_free(port_depth_buffer.ceiling[i]);

      port_depth_buffer.floor[i] = NULL;
      port_depth_buffer.ceiling[i] = NULL;
   }

   port_planes_begin();

   //Clear sector visited flags
   for(int i = 0;i<map->sector_count;i++)
      map->sectors[i].visited = 0;

   //Clear sprites
   RvR_array_length_set(port_sprites, 0);

   //Clear report
   port_report.stack_max = 0;
   port_report.sort_swaps = 0;
   port_report.sort_skips = 0;

   port_cam = cam;
   port_map = map;

#if RVR_PORT_PIXELBYPIXEL
   if(RvR_key_pressed(RVR_PORT_PIXELKEY))
      RvR_render_clear(0);
#endif
}

void RvR_port_draw_report(RvR_port_report *report)
{
   if(report==NULL)
      return;

   *report = port_report;
}

void RvR_port_draw_end(RvR_port_selection *select)
{
   sprites_render(select);
}

const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_floor(int x)
{
   return port_depth_buffer.floor[x];
}

const RvR_port_depth_buffer_entry *RvR_port_depth_buffer_entry_ceiling(int x)
{
   return port_depth_buffer.ceiling[x];
}

RvR_port_depth_buffer_entry *port_depth_buffer_entry_new()
{
   if(port_depth_buffer_entry_pool==NULL)
   {
      RvR_port_depth_buffer_entry *e = RvR_malloc(sizeof(*e) * 256, "RvR_port port_depth_buffer_entry pool");
      memset(e, 0, sizeof(*e) * 256);

      for(int i = 0; i<255; i++)
         e[i].next = &e[i + 1];
      port_depth_buffer_entry_pool = e;
   }

   RvR_port_depth_buffer_entry *e = port_depth_buffer_entry_pool;
   port_depth_buffer_entry_pool = e->next;
   e->next = NULL;

   return e;
}

void port_depth_buffer_entry_free(RvR_port_depth_buffer_entry *ent)
{
   if(ent==NULL)
      return;

   //Find last
   RvR_port_depth_buffer_entry *last = ent;
   while(last->next!=NULL)
      last = last->next;

   last->next = port_depth_buffer_entry_pool;
   port_depth_buffer_entry_pool = ent;
}
//-------------------------------------
