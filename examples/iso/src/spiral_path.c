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
static RvR_fix16 fov_lightgrid[FOV_SIGHT_RADIUS*2][FOV_SIGHT_RADIUS*2][2];
static RvR_fix16 fov_grid[FOV_SIGHT_RADIUS*2+1][FOV_SIGHT_RADIUS*2+1];

static Queue_entry fov_queue[2*FOV_SIGHT_RADIUS*2];
static int fov_queue_tail;
static int fov_queue_head;
//-------------------------------------

//Function prototypes
static void fov_test_mark(int x, int y, int z, RvR_fix16 lit_angle_least, RvR_fix16 lit_angle_greatest, RvR_fix16 angle_min, RvR_fix16 angle_max);
static void fov_mark(int x, int y, int z, RvR_fix16 min, RvR_fix16 max);
static void fov_lit_angle_get(int x, int y, RvR_fix16 *lit_min, RvR_fix16 *lit_max);
static void fov_lit_angle_set(int x, int y, RvR_fix16 lit_min, RvR_fix16 lit_max);

static void fov_child_first(int x, int y, int z, int *childx, int *childy);
static void fov_child_second(int x, int y, int z, int *childx, int *childy);
static void fov_child_third(int x, int y, int z, int *childx, int *childy);

static RvR_fix16 fov_angle_min(int x, int y);
static RvR_fix16 fov_angle_max(int x, int y);
static RvR_fix16 fov_coord_angle(int x, int y);
static RvR_fix16 fov_angle_norm(RvR_fix16 angle);
//-------------------------------------

//Function implementations

void fov_player(Area *a, Entity *e, int oldx, int oldy, int oldz)
{
   //Reset around old location
   for(int z = oldz-1;z<=oldz+1;z++)
   {
      for(int y = oldy-16;y<=oldy+16;y++)
      {
         for(int x = oldx-16;x<=oldx+16;x++)
         {
            area_set_tile(a,x,y,z,tile_set_visible(area_tile(a,x,y,z),0));
         }
      }
   }

   fov_queue_head = fov_queue_tail = 0;

   //Set player position as visible
   int px = e->x;
   int py = e->y;
   int pz = e->z;
   area_set_tile(a,px,py,pz,tile_set_visible(area_tile(a,px,py,pz),1));
   area_set_tile(a,px,py,pz,tile_set_discovered(area_tile(a,px,py,pz),1));

   //test sorrounding squares
   fov_test_mark(1,0,0,0,65536,fov_angle_min(1,0),fov_angle_max(1,0));
   fov_test_mark(0,1,0,0,65536,fov_angle_min(0,1),fov_angle_max(0,1));
   fov_test_mark(-1,0,0,0,65536,fov_angle_min(-1,0),fov_angle_max(-1,0));
   fov_test_mark(0,-1,0,0,65536,fov_angle_min(0,-1),fov_angle_max(0,-1));

   while(fov_queue_head!=fov_queue_tail)
   {
      int cx = fov_queue[fov_queue_head].x;
      int cy = fov_queue[fov_queue_head].y;
      fov_queue_head = (fov_queue_head+1)&(2*FOV_SIGHT_RADIUS*2);
   }
}

static void fov_test_mark(int x, int y, int z, RvR_fix16 lit_angle_least, RvR_fix16 lit_angle_greatest, RvR_fix16 angle_min, RvR_fix16 angle_max)
{
   if(lit_angle_least>lit_angle_greatest)
   {
      fov_mark(x,y,z,angle_min,angle_max);
   }
   else if(angle_max<lit_angle_least||angle_min>lit_angle_greatest)
   {
      return;
   }
   else if(angle_min<=lit_angle_least&&lit_angle_greatest<=angle_max)
   {
      fov_mark(x,y,z,lit_angle_least,lit_angle_greatest);
   }
   else if(angle_min>=lit_angle_least&&lit_angle_greatest>=angle_max)
   {
      fov_mark(x,y,z,angle_min,angle_max);
   }
   else if(angle_min>=lit_angle_least&&lit_angle_greatest<=angle_max)
   {
      fov_mark(x,y,z,angle_min,lit_angle_greatest);
   }
   else if(angle_min<=lit_angle_least&&lit_angle_greatest>=angle_max)
   {
      fov_mark(x,y,z,lit_angle_least,angle_max);
   }
}

static void fov_mark(int x, int y, int z, RvR_fix16 min, RvR_fix16 max)
{
   RvR_fix16 lit_min = 0;
   RvR_fix16 lit_max = 0;
   fov_lit_angle_get(x,y,&lit_min,&lit_max);
   
   if(lit_min==lit_max&&lit_max==0)
   {
      //Not in queue
      fov_lit_angle_set(x,y,min,max);
      fov_queue[fov_queue_tail] = (Queue_entry){.x = x, .y = y};
      fov_queue_tail = (fov_queue_tail+1)%(2*FOV_SIGHT_RADIUS*2);
   }
   else
   {
      if(min<lit_min)
         lit_min = min;
      if(max>lit_max)
         lit_max = max;
      fov_lit_angle_set(x,y,lit_min,lit_max);
   }
}

static void fov_lit_angle_get(int x, int y, RvR_fix16 *lit_min, RvR_fix16 *lit_max)
{
   *lit_min = fov_lightgrid[x+FOV_SIGHT_RADIUS][y+FOV_SIGHT_RADIUS][0];
   *lit_max = fov_lightgrid[x+FOV_SIGHT_RADIUS][y+FOV_SIGHT_RADIUS][1];
}

static void fov_lit_angle_set(int x, int y, RvR_fix16 lit_min, RvR_fix16 lit_max)
{
   fov_lightgrid[x+FOV_SIGHT_RADIUS][y+FOV_SIGHT_RADIUS][0] = lit_min;
   fov_lightgrid[x+FOV_SIGHT_RADIUS][y+FOV_SIGHT_RADIUS][1] = lit_max;
}

static void fov_child_first(int x, int y, int z, int *childx, int *childy)
{
   if(x==0&&y==0)
   {
      *childx = x;
      *childy = y;
   }
   else if(x>=0&&y>0)
   {
      *childx = x+1;
      *childy = y;
   }
}

static void fov_child_second(int x, int y, int z, int *childx, int *childy)
{
}

static void fov_child_third(int x, int y, int z, int *childx, int *childy)
{
}

static RvR_fix16 fov_angle_min(int x, int y)
{
   if(x==0&&y==0)
      return 0;
   if(x>=0&&y>0)
      return fov_coord_angle(x+1,y);
   if(x<0&&y>=0)
      return fov_coord_angle(x+1,y+1);
   if(x<=0&&y<0)
      return fov_coord_angle(x,y+1);
   return fov_coord_angle(x,y);
}

static RvR_fix16 fov_angle_max(int x, int y)
{
   if(x==0&&y==0)
      return 65536;
   if(x>0&&y>=0)
      return fov_coord_angle(x,y+1);
   if(x<=0&&y>0)
      return fov_coord_angle(x,y);
   if(x<0&&y<=0)
      return fov_coord_angle(x+1,y);
   return fov_coord_angle(x+1,y+1);
}

static RvR_fix16 fov_coord_angle(int x, int y)
{
   if(x>FOV_SIGHT_RADIUS||y>FOV_SIGHT_RADIUS||-x>FOV_SIGHT_RADIUS||-y>FOV_SIGHT_RADIUS)
      return fov_angle_norm(RvR_fix16_atan2_slow(y*65536-32768,x*65536-32768));

   if(x>=0&&y>=0)
      return fov_grid[x][y];
   if(x<0&&y>0)
      return 32768-fov_grid[1-x][y];
   if(x>=0&&y<0)
      return 65536-fov_grid[x][1-y];
   return 32768+fov_grid[1-x][1-y];
}

static RvR_fix16 fov_angle_norm(RvR_fix16 angle)
{
   while(angle<0)
      angle+=65536;
   while(angle>65536)
      angle-=65536;
   return angle;
}
//-------------------------------------
