/*
RvnicRaven - portal walls 

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "RvR/RvR_portal.h"
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

//TODO(Captain4LK): we can do a binary search here
//--> speed up RvR_port_wall_previous() by using RvR_port_wall_sector()
uint16_t RvR_port_wall_sector(const RvR_port_map *map, uint16_t wall)
{
   for(int i = 0;i<map->sector_count;i++)
      if(wall>=map->sectors[i].wall_first&&wall<map->sectors[i].wall_first+map->sectors[i].wall_count)
         return (uint16_t)i;

   return RVR_PORT_SECTOR_INVALID;
}

void RvR_port_wall_move(RvR_port_map *map, uint16_t wall, RvR_fix22 x, RvR_fix22 y)
{
   if(map->walls[wall].x==x&&map->walls[wall].y==y)
      return;

   RvR_fix22 old_x = map->walls[wall].x;
   RvR_fix22 old_y = map->walls[wall].y;

   map->walls[wall].x = x;
   map->walls[wall].y = y;
   uint16_t prev = RvR_port_wall_previous(map,wall);

   //wall adjacent
   //-------------------------------------
   uint16_t next = map->walls[wall].portal_wall;
   while(next!=RVR_PORT_WALL_INVALID)
   {
      uint16_t p2 = RvR_port_wall_next(map,next);
      if(map->walls[p2].x==old_x&&map->walls[p2].y==old_y)
      {
         map->walls[p2].x = x;
         map->walls[p2].y = y;
         next = map->walls[p2].portal_wall;
      }
      else
      {
         break;
      }
   }
   //-------------------------------------

   //prev adjacent
   //-------------------------------------
   next = map->walls[prev].portal_wall;
   while(next!=RVR_PORT_WALL_INVALID)
   {
      if(map->walls[next].x==old_x&&map->walls[next].y==old_y)
      {
         map->walls[next].x = x;
         map->walls[next].y = y;
         next = map->walls[RvR_port_wall_previous(map,next)].portal_wall;
      }
      else
      {
         break;
      }
   }
   //-------------------------------------
}

uint16_t RvR_port_wall_first(const RvR_port_map *map, uint16_t wall)
{
   uint16_t cur = wall;
   for(;;)
   {
      if(cur==0)
         return 0;

      if(map->walls[cur-1].p2!=cur)
         return cur;

      cur--;
   }
}

uint16_t RvR_port_wall_insert(RvR_port_map *map, uint16_t w0, RvR_fix22 x, RvR_fix22 y)
{
   uint16_t w1 = map->walls[w0].p2;
   uint16_t w2 = map->walls[w0].portal_wall;
   uint16_t w3 = RVR_PORT_WALL_INVALID;
   uint16_t sector = RvR_port_wall_sector(map,w0);

   //Add wall to org sector
   //-------------------------------------
   map->wall_count++;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
   uint16_t insert = w0+1;

   //Move existing walls to right
   for(int w = map->wall_count-1;w>insert;w--)
      map->walls[w] = map->walls[w-1];

   //Update indices
   for(int i = 0;i<map->wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+i;
      if(wall->p2>=insert)
         wall->p2++;
      if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=insert)
         wall->portal_wall++;
   }
   //Update sector first_wall
   for(int i = 0;i<map->sector_count;i++)
   {
      if(i==RvR_port_wall_sector(map,w0))
         continue;
      RvR_port_sector *sect = map->sectors+i;
      if(sect->wall_first>=insert)
         sect->wall_first++;
   }
   if(w2!=RVR_PORT_WALL_INVALID&&w2>=insert)
      w2++;
   if(w1!=RVR_PORT_WALL_INVALID&&w1>=insert)
      w1++;

   //Insert new wall
   map->walls[insert].x = x;
   map->walls[insert].y = y;
   map->walls[insert].p2 = w1;
   map->walls[insert].portal = map->walls[w0].portal;
   map->walls[insert].portal_wall = RVR_PORT_WALL_INVALID;
   map->walls[insert].flags = 0;
   map->walls[w0].p2 = insert;
   map->sectors[sector].wall_count++;
   //-------------------------------------

   //Add wall to adjacent sector
   //-------------------------------------
   if(w2!=RVR_PORT_WALL_INVALID)
   {
      map->wall_count++;
      map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
      w3 = map->walls[w2].p2;
      uint16_t insert1 = w2+1;
      sector = RvR_port_wall_sector(map,w2);

      //Move existing walls to right
      for(int w = map->wall_count-1;w>insert1;w--)
         map->walls[w] = map->walls[w-1];

      //Update indices
      for(int i = 0;i<map->wall_count;i++)
      {
         RvR_port_wall *wall = map->walls+i;
         if(wall->p2>=insert1)
            wall->p2++;
         if(wall->portal_wall!=RVR_PORT_WALL_INVALID&&wall->portal_wall>=insert1)
            wall->portal_wall++;
      }
      //Update sector first_wall
      for(int i = 0;i<map->sector_count;i++)
      {
         if(i==RvR_port_wall_sector(map,w2))
            continue;
         RvR_port_sector *sect = map->sectors+i;
         if(sect->wall_first>=insert1)
            sect->wall_first++;
      }
      if(insert>=insert1)
         insert++;
      if(w3!=RVR_PORT_WALL_INVALID&&w3>=insert1)
         w3++;

      //Insert new wall
      map->walls[insert1].x = x;
      map->walls[insert1].y = y;
      map->walls[insert1].p2 = w3;
      map->walls[w2].p2 = insert1;
      map->sectors[sector].wall_count++;
      map->walls[RvR_port_wall_previous(map,insert)].portal_wall = insert1;
      map->walls[insert1].portal_wall = RvR_port_wall_previous(map,insert);
      map->walls[insert].portal_wall = RvR_port_wall_previous(map,insert1);
      map->walls[RvR_port_wall_previous(map,insert1)].portal_wall = insert;
      map->walls[insert1].portal = map->walls[w2].portal;
      map->walls[insert1].flags = 0;
   }
   //-------------------------------------

   return insert;
}

uint16_t RvR_port_wall_next(const RvR_port_map *map, uint16_t wall)
{
   return map->walls[wall].p2;
}

uint16_t RvR_port_wall_previous(const RvR_port_map *map, uint16_t wall)
{
   if(wall>0&&map->walls[wall-1].p2==wall)
      return wall-1;

   for(int i = 0;i<map->wall_count;i++)
      if(map->walls[i].p2==wall)
         return (uint16_t)i;

   return RVR_PORT_WALL_INVALID;
}

int RvR_port_wall_winding(const RvR_port_map *map, uint16_t wall)
{
   int64_t sum = 0;
   int cur = wall;
   do
   {
      int64_t x0 = map->walls[cur].x;
      int64_t y0 = map->walls[cur].y;
      int64_t x1 = map->walls[map->walls[cur].p2].x;
      int64_t y1 = map->walls[map->walls[cur].p2].y;
      sum+=(x1-x0)*(y0+y1);
      cur = map->walls[cur].p2;
   }
   while(cur!=wall);
   
   return sum>0;
}

int RvR_port_wall_inside(const RvR_port_map *map, uint16_t wall, RvR_fix22 x, RvR_fix22 y)
{
   RvR_error_check(map!=NULL,"RvR_port_wall_inside","argument 'map' must be non-NULL\n");
   RvR_error_check(wall!=RVR_PORT_SECTOR_INVALID,"RvR_port_wall_inside","invalid wall\n");
   RvR_error_check(wall<map->wall_count,"RvR_port_sector_inside","wall %d out of bounds (%d walls total)\n",wall,map->wall_count);

   //Even-odd rule for checking if inside
   int64_t crossed0 = 0;
   int64_t crossed1 = 0;
   uint16_t cur = wall;
   do
   {
      //Translate, so that (x,y) is at (0,0)
      //--> easier comparisons later
      RvR_port_wall *w0 = map->walls+cur;
      RvR_port_wall *w1 = map->walls+w0->p2;
      int64_t x0 = w0->x-x;
      int64_t y0 = w0->y-y;
      int64_t x1 = w1->x-x;
      int64_t y1 = w1->y-y;

      //We count the amount of walls on the line y = 0, incrementing
      //for every wall left of x = 0

      //Exactly on wall
      if((x0==0&&y0==0)||(x1==0&&y1==0))
         return 1;

      //If signs are not equal, then one of the points
      //is above (x,y), the other below
      if(!RvR_sign_equal(y0,y1))
      {
         //If both on one side
         if(RvR_sign_equal(x0,x1))
            //If both to the left --> increment
            crossed0+=x0<0;
         else
            //One of the x coordinates on the right of line, the other on the left
            //--> more complicated check
            //we use the 2d cross product/perp doct product
            //to compute on which side of the line we are
            //the result is dependant on the sign of y1 (or y0)
            //because we don't know, which of the two coordinates is the upper one
            crossed0+=!RvR_sign_equal(x0*y1-x1*y0,y1);
      }

      //Same thing again, but with
      //half-open interval
      x0--;
      y0--;
      x1--;
      y1--;
      if(!RvR_sign_equal(y0,y1))
      {

         if(RvR_sign_equal(x0,x1))
            crossed1+=x0<0;
         else
            crossed1+=!RvR_sign_equal(x0*y1-x1*y0,y1);
      }

      cur = map->walls[cur].p2;
   }
   while(cur!=wall);

   //If we've crossed an odd number of walls
   //on one of the counting rules, we are
   //inside the sector
   return crossed0&1||crossed1&1;

RvR_err:
   return 0;
}

int RvR_port_wall_subsector(const RvR_port_map *map, uint16_t sector, uint16_t wall)
{
   int subsector = 0;
   for(int i = 0;i<map->sectors[sector].wall_count;i++)
   {
      int iwall = map->sectors[sector].wall_first+i;
      if(iwall==wall)
         break;

      if(map->walls[iwall].p2<iwall)
         subsector++;
   }

   return subsector;
}

uint16_t RvR_port_subsector_length(const RvR_port_map *map, uint16_t wall)
{
   uint16_t len = 1;
   uint16_t cur = map->walls[wall].p2;
   while(cur!=wall)
   {
      cur = map->walls[cur].p2;
      len++;
   }

   return len;
}

uint16_t RvR_port_wall_next_onesided(const RvR_port_map *map, uint16_t wall)
{
   uint16_t p2 = map->walls[wall].p2;
   if(map->walls[p2].portal==RVR_PORT_SECTOR_INVALID)
      return p2;

   uint16_t pw = map->walls[p2].portal_wall;
   for(;;)
   {
      uint16_t pw2 = map->walls[pw].p2;
      if(map->walls[pw2].portal==RVR_PORT_SECTOR_INVALID)
         return pw2;

      pw = map->walls[pw2].portal_wall;
   }
}

uint16_t RvR_port_wall_onesided_length(const RvR_port_map *map, uint16_t wall)
{
   uint16_t len = 1;
   uint16_t cur = RvR_port_wall_next_onesided(map,wall);
   while(cur!=wall)
   {
      cur = RvR_port_wall_next_onesided(map,cur);
      len++;
   }

   return len;
}
//-------------------------------------
