/*
RvnicRaven-portal

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
static int port_point_on_line(RvR_fix22 x, RvR_fix22 y, RvR_fix22 x0, RvR_fix22 y0, RvR_fix22 x1, RvR_fix22 y1);
//-------------------------------------

//Function implementations

//TODO(Captain4LK): we can do a binary search here
//--> speed up RvR_port_wall_previous() by using RvR_port_wall_sector()
uint16_t RvR_port_wall_sector(const RvR_port_map *map, uint16_t wall)
{
   if(wall==RVR_PORT_WALL_INVALID)
      return RVR_PORT_SECTOR_INVALID;

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

uint16_t RvR_port_wall_make_first(RvR_port_map *map, uint16_t wall)
{
   if(wall==RVR_PORT_WALL_INVALID)
      return wall;

   uint16_t first = RvR_port_subsector_first(map,wall);
   uint16_t len = RvR_port_subsector_length(map,wall);
   uint16_t sector = RvR_port_wall_sector(map,wall);

   //Rotate
   //-------------------------------------
   int rot_amount = len-(wall-first);
   int rot_start = len-rot_amount;

   //Reverse all
   for(int i = 0;i<len/2;i++)
   {
      RvR_port_wall tmp = map->walls[first+i];
      map->walls[first+i] = map->walls[first+len-i-1];
      map->walls[first+len-i-1] = tmp;
   }

   //Reverse last rot_start
   for(int i = 0;i<rot_start/2;i++)
   {
      RvR_port_wall tmp = map->walls[first+rot_amount+i];
      map->walls[first+rot_amount+i] = map->walls[first+len-i-1];
      map->walls[first+len-i-1] = tmp;
   }

   //Reverse until rot_amount
   for(int i = 0;i<rot_amount/2;i++)
   {
      RvR_port_wall tmp = map->walls[first+i];
      map->walls[first+i] = map->walls[first+rot_amount-i-1];
      map->walls[first+rot_amount-i-1] = tmp;
   }

   //Fix indices
   for(int i = 0;i<len;i++)
      map->walls[first+i].p2 = (uint16_t)((map->walls[first+i].p2-first+rot_amount)%len+first);
   //-------------------------------------

   //Fix portals
   for(int i = 0;i<len;i++)
   {
      RvR_port_wall *w = map->walls+first+i;
      if(w->portal_wall!=RVR_PORT_WALL_INVALID)
      {
         map->walls[w->portal_wall].portal_wall = first+(uint16_t)i;
         map->walls[w->portal_wall].portal = sector;
      }
   }

   return wall;
}

uint16_t RvR_port_wall_first(const RvR_port_map *map, uint16_t wall)
{
   if(wall==RVR_PORT_WALL_INVALID)
      return wall;

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
   map->walls[insert] = map->walls[insert-1];
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
      map->walls[insert1] = map->walls[insert1-1];
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

   int crossed = 0;
   uint16_t cur = wall;
   do
   {
      RvR_port_wall *w0 = map->walls+cur;
      RvR_port_wall *w1 = map->walls+w0->p2;
      int64_t x0 = w0->x;
      int64_t y0 = w0->y;
      int64_t x1 = w1->x;
      int64_t y1 = w1->y;

      if(port_point_on_line(x,y,w0->x,w0->y,w1->x,w1->y))
         return 1;

      //Below point
      if(RvR_min(y0,y1)>=y)
         continue;

      //Above point
      if(RvR_max(y0,y1)<y)
         continue;

      //Left of point
      if(RvR_max(x0,x1)<x)
         continue;

      //Horizontal line
      if(y0==y1)
         continue;

      //Vertical line
      if(x0==x1)
      {
         crossed++;
         continue;
      }
      
      if(y1-y0>0)
      {
         if(x*(y1-y0)<=(y-y0)*(x1-x0)+x0*(y1-y0))
            crossed++;
      }
      else
      {
         if(x*(y1-y0)>=(y-y0)*(x1-x0)+x0*(y1-y0))
            crossed++;
      }

      cur = map->walls[cur].p2;
   }
   while(cur!=wall);

   return crossed&1;

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

uint16_t RvR_port_subsector_first(const RvR_port_map *map, uint16_t wall)
{
   if(map->walls[wall].p2<wall)
      return map->walls[wall].p2;

   uint16_t cur = map->walls[wall].p2;
   while(cur!=wall)
   {
      if(map->walls[cur].p2<cur)
         return map->walls[cur].p2;
      cur = map->walls[cur].p2;
   }

   return wall;
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

void RvR_port_wall_delete(RvR_port_map *map, uint16_t wall)
{
   uint16_t sector = RvR_port_wall_sector(map,wall);
   uint16_t other = RVR_PORT_WALL_INVALID;
   if(other!=RVR_PORT_WALL_INVALID)
      other = map->walls[map->walls[wall].portal_wall].p2;
   uint16_t sector_portal = RVR_PORT_SECTOR_INVALID;
   if(other!=RVR_PORT_WALL_INVALID)
      sector_portal = RvR_port_wall_sector(map,other);

   //Subsector too small, use sector_delete instead
   if(RvR_port_subsector_length(map,wall)<=3)
      return;
   if(other!=RVR_PORT_WALL_INVALID&&RvR_port_subsector_length(map,other)<=3)
      return;

   uint16_t prev = RvR_port_wall_previous(map,wall);
   map->walls[prev].p2 = map->walls[wall].p2;

   //Move existing walls to left
   for(int w = wall;w<map->wall_count-1;w++)
      map->walls[w] = map->walls[w+1];

   //Update indices
   for(int w = 0;w<map->wall_count;w++)
   {
      if(map->walls[w].p2>=wall)
         map->walls[w].p2--;
      if(map->walls[w].portal_wall!=RVR_PORT_WALL_INVALID&&map->walls[w].portal_wall>=wall)
         map->walls[w].portal_wall--;
   }
   if(other!=RVR_PORT_WALL_INVALID&&other>=wall)
      other--;

   //Update sector references
   for(int s = 0;s<map->sector_count;s++)
   {
      if(map->sectors[s].wall_first>wall)
         map->sectors[s].wall_first--;
   }
   
   map->wall_count--;
   map->sectors[sector].wall_count--;

   if(other!=RVR_PORT_WALL_INVALID)
   {
      prev = RvR_port_wall_previous(map,other);
      map->walls[prev].p2 = map->walls[other].p2;

      //Move existing walls to left
      for(int w = other;w<map->wall_count-1;w++)
         map->walls[w] = map->walls[w+1];

      //Update indices
      for(int w = 0;w<map->wall_count;w++)
      {
         if(map->walls[w].p2>=other)
            map->walls[w].p2--;
         if(map->walls[w].portal_wall!=RVR_PORT_WALL_INVALID&&map->walls[w].portal_wall>=other)
            map->walls[w].portal_wall--;
      }

      //Update sector references
      for(int s = 0;s<map->sector_count;s++)
      {
         if(map->sectors[s].wall_first>other)
            map->sectors[s].wall_first--;
      }
      
      map->wall_count--;
      map->sectors[sector_portal].wall_count--;
   }

   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map walls grow");
}

static int port_point_on_line(RvR_fix22 x, RvR_fix22 y, RvR_fix22 x0, RvR_fix22 y0, RvR_fix22 x1, RvR_fix22 y1)
{
   //If 2d cross product is zero, point is on line
   int64_t cross = (int64_t)(x-x0)*(y1-y0)-(int64_t)(y-y0)*(x1-x0);
   if(cross!=0)
      return 0;

   //Point on line, but may not be inbetween p0 and p1
   if(RvR_abs(x1-x0)>=RvR_abs(y1-y0))
   {
      if(x1-x0>0)
         return x0<=x&&x<=x1;
      return x1<=x&&x<=x0;
   }
   else
   {
      if(y1-y0>0)
         return y0<=y&&y<=y1;
      return y1<=y&&y<=y0;
   }
}
//-------------------------------------
