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
static RvR_fix16 fov_lightgrid[FOV_SIGHT_RADIUS*2][FOV_SIGHT_RADIUS*2];

static Queue_entry fov_queue[2*FOV_SIGHT_RADIUS*2];
static int fov_queue_tail;
//-------------------------------------

//Function prototypes
static void fov_test_mark(int x, int y, int z, RvR_fix16 lit_angle_least, RvR_fix16 lit_angle_greatest, RvR_fix16 angle_min, RvR_fix16 angle_max);
static void fov_mark(int x, int y, int z, RvR_fix16 min, RvR_fix16 max);
static void fov_lit_angle_get(int x, int y, RvR_fix16 *lit_min, RvR_fix16 *lit_max);
static void fov_lit_angle_set(int x, int y, RvR_fix16 lit_min, RvR_fix16 lit_max);
static RvR_fix16 fov_angle_min(int x, int y);
static RvR_fix16 fov_angle_max(int x, int y);
static RvR_fix16 fov_coord_angle(int x, int y);
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

   //Set player position as visible
   int px = e->x;
   int py = e->y;
   int pz = e->z;
   area_set_tile(a,px,py,pz,tile_set_visible(area_tile(a,px,py,pz),1));
   area_set_tile(a,px,py,pz,tile_set_discovered(area_tile(a,px,py,pz),1));

   //test sorrounding squares
   fov_test_mark(1,0,0,2*65536,);
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
   *lit_min = fov_lightgrid[(y+FOV_SIGHT_RADIUS)*FOV_SIGHT_RADIUS+x+FOV_SIGHT_RADIUS][0];
   *lit_max = fov_lightgrid[(y+FOV_SIGHT_RADIUS)*FOV_SIGHT_RADIUS+x+FOV_SIGHT_RADIUS][1];
}

static void fov_lit_angle_set(int x, int y, RvR_fix16 lit_min, RvR_fix16 lit_max)
{
   fov_lightgrid[(y+FOV_SIGHT_RADIUS)*FOV_SIGHT_RADIUS+x+FOV_SIGHT_RADIUS][0] = lit_min;
   fov_lightgrid[(y+FOV_SIGHT_RADIUS)*FOV_SIGHT_RADIUS+x+FOV_SIGHT_RADIUS][1] = lit_max;
}

static RvR_fix16 fov_angle_min(int x, int y)
{
   if(x==0&&y==0)
      return 0;
   if(x>=0&&y>0)
}

static RvR_fix16 fov_angle_max(int x, int y)
{
}

static RvR_fix16 fov_coord_angle(int x, int y)
{
   if(x>FOV_SIGHT_RADIUS||y>FOV_SIGHT_RADIUS||-x>FOV_SIGHT_RADIUS||-y>FOV_SIGHT_RADIUS)
      return 
}
//-------------------------------------
