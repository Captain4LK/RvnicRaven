/*
RvnicRaven - iso roguelike

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
//-------------------------------------

//Internal includes
#include "spiral_path.h"
#include "area.h"
#include "tile.h"
//-------------------------------------

//#defines
#define FOV_SIGHT_RADIUS 16
//-------------------------------------

//Typedefs
typedef struct
{
   int x;
   int y;
}Queue_entry;
//-------------------------------------

//Variables
static RvR_fix16 fov_lightgrid[FOV_SIGHT_RADIUS * 2][FOV_SIGHT_RADIUS * 2][2];
static RvR_fix16 fov_grid[FOV_SIGHT_RADIUS * 2 + 1][FOV_SIGHT_RADIUS * 2 + 1];

static Queue_entry fov_queue[2 * FOV_SIGHT_RADIUS * 2];
static int fov_queue_tail;
static int fov_queue_head;

static int fov_initialized = 0;
//-------------------------------------

//Function prototypes
static void fov_test_mark(int x, int y, RvR_fix16 lit_angle_least, RvR_fix16 lit_angle_greatest, RvR_fix16 angle_min, RvR_fix16 angle_max);
static void fov_mark(int x, int y, RvR_fix16 min, RvR_fix16 max);
static void fov_lit_angle_get(int x, int y, RvR_fix16 *lit_min, RvR_fix16 *lit_max);
static void fov_lit_angle_set(int x, int y, RvR_fix16 lit_min, RvR_fix16 lit_max);

static int fov_transparent(Area *a, Entity *e, int x, int y, int z);

static void fov_child_first(int x, int y, int *childx, int *childy);
static void fov_child_second(int x, int y, int *childx, int *childy);
static void fov_child_third(int x, int y, int *childx, int *childy);

static RvR_fix16 fov_angle_min(int x, int y);
static RvR_fix16 fov_angle_max(int x, int y);
static RvR_fix16 fov_angle_norm(RvR_fix16 angle);
static RvR_fix16 fov_angle_outer(int x, int y);
static RvR_fix16 fov_angle_outer2(int x, int y);
static RvR_fix16 fov_coord_angle(int x, int y);
static int fov_in_arc(int x, int y, RvR_fix16 arc_start, RvR_fix16 arc_end);

static void fov_init();
//-------------------------------------

//Function implementations

void fov_player(Area *a, Entity *e, int oldx, int oldy, int oldz)
{
   if(!fov_initialized)
      fov_init();

   //Reset around old location
   for(int z = oldz - 4; z<=oldz + 4; z++)
   {
      for(int y = oldy - 16; y<=oldy + 16; y++)
      {
         for(int x = oldx - 16; x<=oldx + 16; x++)
         {
            area_set_tile(a, x, y, z, tile_set_visible(area_tile(a, x, y, z), 0, 0));
         }
      }
   }
   int radius = 8;

   fov_queue_head = fov_queue_tail = 0;

   //Set player position as visible
   int px = e->x;
   int py = e->y;
   int pz = e->z;
   area_set_tile(a, px, py, pz, tile_set_visible_wall(tile_set_discovered_wall(area_tile(a, px, py, pz), 1),1));
   area_set_tile(a, px, py, pz + 1, tile_set_visible_floor(tile_set_discovered_floor(area_tile(a, px, py, pz + 1), 1),1));

   //Discover up
   int radius_cur = 0;
   for(int z = 1;z*z*4<radius*radius-radius_cur;z++)
   {
      uint32_t tile0 = area_tile(a,px,py,pz-z+1);
      uint32_t tile1 = area_tile(a,px,py,pz-z);

      if(tile_has_wall(tile0))
         break;

      tile0 = tile_set_discovered_floor(tile_set_visible_floor(tile0,1),1);
      area_set_tile(a,px,py,pz-z+1,tile0);
      if(tile_has_floor(tile0))
         break;

      tile1 = tile_set_discovered_wall(tile_set_visible_wall(tile1,1),1);
      area_set_tile(a,px,py,pz-z,tile1);
   }

   //Discover down
   for(int z = 1;z*z*4<radius*radius-radius_cur;z++)
   {
      uint32_t tile0 = area_tile(a,px,py,pz+z-1);
      uint32_t tile1 = area_tile(a,px,py,pz+z);

      if(tile_has_wall(tile0))
         break;

      tile1 = tile_set_discovered_floor(tile_set_visible_floor(tile1,1),1);
      area_set_tile(a,px,py,pz+z,tile1);
      if(tile_has_floor(tile1))
         break;

      tile1 = tile_set_discovered_wall(tile_set_visible_wall(tile1,1),1);
      area_set_tile(a,px,py,pz+z,tile1);
   }

   //test sorrounding squares
   fov_test_mark(1, 0, 0, 65536, fov_angle_min(1, 0), fov_angle_max(1, 0));
   fov_test_mark(0, 1, 0, 65536, fov_angle_min(0, 1), fov_angle_max(0, 1));
   fov_test_mark(-1, 0, 0, 65536, fov_angle_min(-1, 0), fov_angle_max(-1, 0));
   fov_test_mark(0, -1, 0, 65536, fov_angle_min(0, -1), fov_angle_max(0, -1));

   while(fov_queue_head!=fov_queue_tail)
   {
      int cx = fov_queue[fov_queue_head].x;
      int cy = fov_queue[fov_queue_head].y;
      fov_queue_head = (fov_queue_head + 1) % (2 * FOV_SIGHT_RADIUS * 2);

      int child0x, child0y;
      int child1x, child1y;
      int child2x, child2y;
      fov_child_first(cx, cy, &child0x, &child0y);
      fov_child_second(cx, cy, &child1x, &child1y);
      fov_child_third(cx, cy, &child2x, &child2y);

      RvR_fix16 angle_least = fov_angle_min(cx, cy);
      RvR_fix16 angle_outer = fov_angle_outer(cx, cy);
      RvR_fix16 angle_outer2 = fov_angle_outer2(cx, cy);
      RvR_fix16 angle_greatest = fov_angle_max(cx, cy);

      RvR_fix16 lit_angle_least;
      RvR_fix16 lit_angle_greatest;
      fov_lit_angle_get(cx, cy, &lit_angle_least, &lit_angle_greatest);

      fov_lit_angle_set(cx, cy, 0, 0);

      if(cx * cx + cy * cy<=radius * radius&&fov_in_arc(cx, cy, 0, 65536))
      {
         area_set_tile(a, px + cx, py + cy, pz, tile_set_visible_wall(area_tile(a, px + cx, py + cy, pz), 1));
         area_set_tile(a, px + cx, py + cy, pz, tile_set_discovered_wall(area_tile(a, px + cx, py + cy, pz), 1));

         area_set_tile(a, px + cx, py + cy, pz + 1, tile_set_visible_floor(area_tile(a, px + cx, py + cy, pz + 1), 1));
         area_set_tile(a, px + cx, py + cy, pz + 1, tile_set_discovered_floor(area_tile(a, px + cx, py + cy, pz + 1), 1));

         //Discover up
         radius_cur = cx*cx+cy*cy;
         for(int z = 1;z*z*4<radius*radius-radius_cur;z++)
         {
            uint32_t tile0 = area_tile(a,px+cx,py+cy,pz-z+1);
            uint32_t tile1 = area_tile(a,px+cx,py+cy,pz-z);

            if(tile_has_wall(tile0))
               break;

            tile0 = tile_set_discovered_floor(tile_set_visible_floor(tile0,1),1);
            area_set_tile(a,px+cx,py+cy,pz-z+1,tile0);
            if(tile_has_floor(tile0))
               break;

            tile1 = tile_set_discovered_wall(tile_set_visible_wall(tile1,1),1);
            area_set_tile(a,px+cx,py+cy,pz-z,tile1);
         }

         //Discover down
         for(int z = 1;z*z*4<radius*radius-radius_cur;z++)
         {
            uint32_t tile0 = area_tile(a,px+cx,py+cy,pz+z-1);
            uint32_t tile1 = area_tile(a,px+cx,py+cy,pz+z);

            if(tile_has_wall(tile0))
               break;

            tile1 = tile_set_discovered_floor(tile_set_visible_floor(tile1,1),1);
            area_set_tile(a,px+cx,py+cy,pz+z,tile1);
            if(tile_has_floor(tile1))
               break;

            tile1 = tile_set_discovered_wall(tile_set_visible_wall(tile1,1),1);
            area_set_tile(a,px+cx,py+cy,pz+z,tile1);
         }

         if(fov_transparent(a, e, px + cx, py + cy, pz))
         {
            fov_test_mark(child0x, child0y, lit_angle_least, lit_angle_greatest, angle_least, angle_outer);
            if(angle_outer2!=0)
            {
               fov_test_mark(child1x, child1y, lit_angle_least, lit_angle_greatest, angle_outer, angle_outer2);
               fov_test_mark(child2x, child2y, lit_angle_least, lit_angle_greatest, angle_outer2, angle_greatest);
            }
            else
            {
               fov_test_mark(child1x, child1y, lit_angle_least, lit_angle_greatest, angle_outer, angle_greatest);
            }
         }
         else if(lit_angle_least==angle_least)
         {
            fov_mark(child0x, child0y, angle_least, angle_least);
         }
      }
   }
}

static void fov_test_mark(int x, int y, RvR_fix16 lit_angle_least, RvR_fix16 lit_angle_greatest, RvR_fix16 angle_min, RvR_fix16 angle_max)
{
   if(lit_angle_least>lit_angle_greatest)
      fov_mark(x, y, angle_min, angle_max);
   else if(angle_max<lit_angle_least||angle_min>lit_angle_greatest)
      return;
   else if(angle_min<=lit_angle_least&&lit_angle_greatest<=angle_max)
      fov_mark(x, y, lit_angle_least, lit_angle_greatest);
   else if(angle_min>=lit_angle_least&&lit_angle_greatest>=angle_max)
      fov_mark(x, y, angle_min, angle_max);
   else if(angle_min>=lit_angle_least&&lit_angle_greatest<=angle_max)
      fov_mark(x, y, angle_min, lit_angle_greatest);
   else if(angle_min<=lit_angle_least&&lit_angle_greatest>=angle_max)
      fov_mark(x, y, lit_angle_least, angle_max);
}

static void fov_mark(int x, int y, RvR_fix16 min, RvR_fix16 max)
{
   RvR_fix16 lit_min = 0;
   RvR_fix16 lit_max = 0;
   fov_lit_angle_get(x, y, &lit_min, &lit_max);

   if(lit_min==0&&lit_max==0)
   {
      //Not in queue
      fov_lit_angle_set(x, y, min, max);
      fov_queue[fov_queue_tail] = (Queue_entry){
         .x = x, .y = y
      };
      fov_queue_tail = (fov_queue_tail + 1) % (2 * FOV_SIGHT_RADIUS * 2);
   }
   else
   {
      if(min<lit_min)
         lit_min = min;
      if(max>lit_max)
         lit_max = max;
      fov_lit_angle_set(x, y, lit_min, lit_max);
   }
}

static void fov_lit_angle_get(int x, int y, RvR_fix16 *lit_min, RvR_fix16 *lit_max)
{
   *lit_min = fov_lightgrid[x + FOV_SIGHT_RADIUS][y + FOV_SIGHT_RADIUS][0];
   *lit_max = fov_lightgrid[x + FOV_SIGHT_RADIUS][y + FOV_SIGHT_RADIUS][1];
}

static void fov_lit_angle_set(int x, int y, RvR_fix16 lit_min, RvR_fix16 lit_max)
{
   fov_lightgrid[x + FOV_SIGHT_RADIUS][y + FOV_SIGHT_RADIUS][0] = lit_min;
   fov_lightgrid[x + FOV_SIGHT_RADIUS][y + FOV_SIGHT_RADIUS][1] = lit_max;
}

static int fov_transparent(Area *a, Entity *e, int x, int y, int z)
{
   return !(tile_has_wall(area_tile(a, x, y, z))||tile_is_slope(area_tile(a,x,y,z)));
}

static void fov_child_first(int x, int y, int *childx, int *childy)
{
   if(x==0&&y==0) { *childx = x; *childy = y; }
   else if(x>=0&&y>0) { *childx = x + 1; *childy = y; }
   else if(x<0&&y>=0) { *childx = x; *childy = y + 1; }
   else if(x<=0&&y<0) { *childx = x - 1; *childy = y; }
   else { *childx = x; *childy = y - 1; }
}

static void fov_child_second(int x, int y, int *childx, int *childy)
{
   if(x==0&&y==0) { *childx = x; *childy = y; }
   else if(x>=0&&y>0) { *childx = x; *childy = y + 1; }
   else if(x<0&&y>=0) { *childx = x - 1; *childy = y; }
   else if(x<=0&&y<0) { *childx = x; *childy = y - 1; }
   else { *childx = x + 1; *childy = y; }
}

static void fov_child_third(int x, int y, int *childx, int *childy)
{
   if(x!=0&&y!=0) { *childx = 0; *childy = 0; }
   else if(x>0) { *childx = x; *childy = y + 1; }
   else if(x<0) { *childx = x; *childy = y - 1; }
   else if(y>0) { *childx = x - 1; *childy = y; }
   else if(y<0) { *childx = x + 1; *childy = y; }
   else { *childx = 0; *childy = 0; }
}

static RvR_fix16 fov_angle_min(int x, int y)
{
   if(x==0&&y==0)
      return 0;
   if(x>=0&&y>0)
      return fov_coord_angle(x + 1, y);
   if(x<0&&y>=0)
      return fov_coord_angle(x + 1, y + 1);
   if(x<=0&&y<0)
      return fov_coord_angle(x, y + 1);
   return fov_coord_angle(x, y);
}

static RvR_fix16 fov_angle_max(int x, int y)
{
   if(x==0&&y==0)
      return 65536;
   if(x>0&&y>=0)
      return fov_coord_angle(x, y + 1);
   if(x<=0&&y>0)
      return fov_coord_angle(x, y);
   if(x<0&&y<=0)
      return fov_coord_angle(x + 1, y);
   return fov_coord_angle(x + 1, y + 1);
}

static RvR_fix16 fov_coord_angle(int x, int y)
{
   if(x>FOV_SIGHT_RADIUS||y>FOV_SIGHT_RADIUS||-x>FOV_SIGHT_RADIUS||-y>FOV_SIGHT_RADIUS)
      return fov_angle_norm(RvR_fix16_atan2_slow(y * 65536 - 32768, x * 65536 - 32768));

   if(x>=0&&y>=0)
      return fov_grid[x][y];
   if(x<0&&y>0)
      return 32768 - fov_grid[1 - x][y];
   if(x>=0&&y<0)
      return 65536 - fov_grid[x][1 - y];
   return 32768 + fov_grid[1 - x][1 - y];
}

static RvR_fix16 fov_angle_norm(RvR_fix16 angle)
{
   while(angle<0)
      angle += 65536;
   while(angle>65536)
      angle -= 65536;
   return angle;
}

static RvR_fix16 fov_angle_outer(int x, int y)
{
   if(x==0&&y==0)
      return 0;
   if(x>=0&&y>0)
      return fov_coord_angle(x + 1, y + 1);
   if(x<0&&y>=0)
      return fov_coord_angle(x, y + 1);
   if(x<=0&&y<0)
      return fov_coord_angle(x, y);
   return fov_coord_angle(x + 1, y);
}

static RvR_fix16 fov_angle_outer2(int x, int y)
{
   if(x!=0&&y!=0)
      return 0;
   if(x>0)
      return fov_coord_angle(x + 1, y + 1);
   if(x<0)
      return fov_coord_angle(x, y);
   if(y>0)
      return fov_coord_angle(x, y + 1);
   if(y<0)
      return fov_coord_angle(x + 1, y);
   return 0;
}

static int fov_in_arc(int x, int y, RvR_fix16 arc_start, RvR_fix16 arc_end)
{
   if(arc_start>arc_end)
      return fov_angle_min(x, y)<arc_start||fov_angle_max(x, y)<arc_start||fov_angle_min(x, y)>arc_end||fov_angle_max(x, y)>arc_end;
   return fov_angle_max(x, y)>arc_start||fov_angle_min(x, y)<arc_end;
}

static void fov_init()
{
   fov_initialized = 1;

   for(int y = 0; y<FOV_SIGHT_RADIUS * 2 + 1; y++)
      for(int x = 0; x<FOV_SIGHT_RADIUS * 2 + 1; x++)
         fov_grid[x][y] = fov_angle_norm(RvR_fix16_atan2_slow(y * 65536 - 32768, x * 65536 - 32768));
}
//-------------------------------------
