/*
RvnicRaven - portal walls 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
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

void RvR_port_wall_move(RvR_port_map *map, int16_t wall, RvR_fix16 x, RvR_fix16 y)
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

int16_t RvR_port_wall_first(RvR_port_map *map, int16_t wall)
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

int16_t RvR_port_wall_append(RvR_port_map *map, int16_t sector, RvR_fix16 x, RvR_fix16 y)
{
   int16_t first = 0;
   if(map->walls[map->sectors[sector].wall_first+map->sectors[sector].wall_count-1].p2==map->sectors[sector].wall_first+map->sectors[sector].wall_count)
      first = RvR_port_wall_first(map,map->sectors[sector].wall_first+map->sectors[sector].wall_count-1);
   else
      first = map->sectors[sector].wall_first+map->sectors[sector].wall_count;

   //Check if point overlap
   for(int16_t i = first;i<map->sectors[sector].wall_first+map->sectors[sector].wall_count;i++)
   {
      if(map->walls[i].x==x&&map->walls[i].y==y)
      {
         map->walls[map->sectors[sector].wall_first+map->sectors[sector].wall_count-1].p2 = first;
         return i;
      }
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
      if(wall->portal>=insert)
         wall->portal++;
      if(wall->p2>=insert)
         wall->p2++;
      if(wall->join>=insert)
         wall->join++;
   }

   //Insert new wall
   map->walls[insert].x = x;
   map->walls[insert].y = y;
   map->walls[insert].p2 = insert+1;
   map->walls[insert].join = -1;
   map->walls[insert].portal = -1;
   map->walls[insert].flags = 0;
   //-------------------------------------

   return insert;
}

int16_t RvR_port_wall_insert(RvR_port_map *map, int16_t w0, RvR_fix16 x, RvR_fix16 y)
{
}
//-------------------------------------
