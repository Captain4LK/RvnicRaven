/*
RvnicRaven - portal walls 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
int16_t RvR_port_wall_sector(const RvR_port_map *map, int16_t wall)
{
   for(int i = 0;i<map->sector_count;i++)
      if(wall>=map->sectors[i].wall_first&&wall<map->sectors[i].wall_first+map->sectors[i].wall_count)
         return i;

   return -1;
}

void RvR_port_wall_move(RvR_port_map *map, int16_t wall, RvR_fix22 x, RvR_fix22 y)
{
   map->walls[wall].x = x;
   map->walls[wall].y = y;
   int16_t cur = map->walls[wall].join;
   while(cur!=wall&&cur>=0)
   {
      map->walls[cur].x = x;
      map->walls[cur].y = y;
      cur = map->walls[cur].join;
   }
}

int16_t RvR_port_wall_first(const RvR_port_map *map, int16_t wall)
{
   int16_t cur = wall;
   for(;;)
   {
      if(cur==0)
         return 0;

      if(map->walls[cur-1].p2!=cur)
         return cur;

      cur--;
   }
}

int16_t RvR_port_wall_append(RvR_port_map *map, int16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   int16_t first = -1;
   if(map->walls[map->sectors[sector].wall_first+map->sectors[sector].wall_count-1].p2==-1)
      first = RvR_port_wall_first(map,map->sectors[sector].wall_first+map->sectors[sector].wall_count-1);

   //Check if point overlaps with first point of polygon
   if(first>=0&&map->walls[first].x==x&&map->walls[first].y==y)
   {
      map->walls[map->sectors[sector].wall_first+map->sectors[sector].wall_count-1].p2 = first;
      RvR_port_sector_fix_winding(map,sector);
      return first;
   }

   //Add wall
   //-------------------------------------
   map->wall_count++;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
   int16_t insert = map->sectors[sector].wall_first+map->sectors[sector].wall_count;

   //Move existing walls to right
   for(int16_t w = map->wall_count-1;w>insert;w--)
      map->walls[w] = map->walls[w-1];

   //Update indices
   for(int i = 0;i<map->wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+i;
      if(wall->p2>=insert)
         wall->p2++;
      if(wall->join>=insert)
         wall->join++;
   }
   //Update sector first_wall
   for(int i = 0;i<map->sector_count;i++)
   {
      RvR_port_sector *sect = map->sectors+i;
      if(sect->wall_first>=insert)
         sect->wall_first++;
   }

   //Insert new wall
   map->walls[insert].x = x;
   map->walls[insert].y = y;
   map->walls[insert].p2 = -1;
   if(first>=0)
      map->walls[insert-1].p2 = insert;
   map->walls[insert].join = -1;
   map->walls[insert].portal = -1;
   map->walls[insert].flags = 0;
   map->sectors[sector].wall_count++;
   //-------------------------------------

   return insert;
}

int16_t RvR_port_wall_insert(RvR_port_map *map, int16_t w0, RvR_fix22 x, RvR_fix22 y)
{
   int16_t w1 = map->walls[w0].p2;

   //Check for second wall pair
   int16_t cur0 = map->walls[w0].join;
   int16_t w2 = -1;
   int16_t w3 = -1;
   while(cur0>=0&&cur0!=w0)
   {

      int16_t cur1 = map->walls[w1].join;
      while(cur1>=0&&cur1!=w1)
      {
         if(map->walls[cur1].p2==cur0)
         {
            w2 = cur1;
            break;
         }
         cur1 = map->walls[cur1].join;
      }
      if(w2>=0)
         break;

      cur0 = map->walls[cur0].join;
   }

   //Add wall to org sector
   //-------------------------------------
   map->wall_count++;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
   int16_t insert = w0+1;

   //Move existing walls to right
   for(int16_t w = map->wall_count-1;w>insert;w--)
      map->walls[w] = map->walls[w-1];

   //Update indices
   for(int i = 0;i<map->wall_count;i++)
   {
      RvR_port_wall *wall = map->walls+i;
      if(wall->p2>=insert)
         wall->p2++;
      if(wall->join>=insert)
         wall->join++;
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
   if(w2>=insert)
      w2++;
   if(w1>=insert)
      w1++;

   //Insert new wall
   int16_t sector = RvR_port_wall_sector(map,insert);
   map->walls[insert].x = x;
   map->walls[insert].y = y;
   map->walls[insert].p2 = w1;
   map->walls[insert].join = -1;
   map->walls[insert].portal = map->walls[w0].portal;
   map->walls[insert].flags = 0;
   map->walls[w0].p2 = insert;
   map->sectors[sector].wall_count++;
   //-------------------------------------
   
   //Add wall to adjacent sector
   if(w2>=0)
   {
      map->wall_count++;
      map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
      w3 = map->walls[w2].p2;
      int16_t insert1 = w2+1;

      //Move existing walls to right
      for(int16_t w = map->wall_count-1;w>insert1;w--)
         map->walls[w] = map->walls[w-1];

      //Update indices
      for(int i = 0;i<map->wall_count;i++)
      {
         RvR_port_wall *wall = map->walls+i;
         if(wall->p2>=insert1)
            wall->p2++;
         if(wall->join>=insert1)
            wall->join++;
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
      if(w3>=insert1)
         w3++;

      //Insert new wall
      sector = RvR_port_wall_sector(map,insert1);
      map->walls[insert1].x = x;
      map->walls[insert1].y = y;
      map->walls[insert1].p2 = w3;
      map->walls[insert1].join = insert;
      map->walls[insert].join = insert1;
      map->walls[insert1].portal = map->walls[w2].portal;
      map->walls[insert1].flags = 0;
      map->walls[w2].p2 = insert1;
      map->sectors[sector].wall_count++;
   }

   return insert;
}

int16_t RvR_port_wall_next(const RvR_port_map *map, int16_t wall)
{
   return map->walls[wall].p2;
}

int16_t RvR_port_wall_previous(const RvR_port_map *map, int16_t wall)
{
   if(wall>0&&map->walls[wall-1].p2==wall)
      return wall-1;

   for(int i = 0;i<map->wall_count;i++)
      if(map->walls[i].p2==wall)
         return i;

   return -1;
}

int16_t RvR_port_wall_join_previous(const RvR_port_map *map, int16_t wall)
{
   if(map->walls[wall].join<0)
      return -1;

   int16_t cur = map->walls[wall].join;
   for(int i = 0;i<1024&&map->walls[cur].join!=wall;i++)
   {
      cur = map->walls[cur].join;
      if(cur==-1)
         return -1;
      if(map->walls[cur].join==wall)
         return cur;
   }

   printf("hit %d\n",wall);
   for(int i = 0;i<map->wall_count;i++)
      if(map->walls[i].join==wall)
         puts("FOUND");


   return cur;
}

int16_t RvR_port_wall_winding(const RvR_port_map *map, int16_t wall)
{
   int64_t sum = 0;
   int cur = map->walls[wall].p2;

   while(cur!=wall)
   {
      int64_t x0 = map->walls[cur].x;
      int64_t y0 = map->walls[cur].y;
      int64_t x1 = map->walls[map->walls[cur].p2].x;
      int64_t y1 = map->walls[map->walls[cur].p2].y;
      sum+=(x1-x0)*(y0+y1);
      cur = map->walls[cur].p2;
   }
   
   return sum>0;
}

int RvR_port_wall_subsector(const RvR_port_map *map, int16_t sector, int16_t wall)
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

void RvR_port_wall_join(RvR_port_map *map, int16_t wall, int16_t join)
{
   if(wall<0||join<0)
      return;

   if(map->walls[wall].join<0)
   {
      map->walls[wall].join = join;
      map->walls[join].join = wall;
      return;
   }

   int16_t cur = map->walls[wall].join;
   while(cur!=wall&&cur!=-1)
   {
      if(cur==join)
         return;
      cur = map->walls[cur].join;
   }

   int16_t next = map->walls[wall].join;
   map->walls[wall].join = join;
   map->walls[join].join = next;
}
//-------------------------------------
