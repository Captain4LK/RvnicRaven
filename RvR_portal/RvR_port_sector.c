/*
RvnicRaven - portal sectors

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

int RvR_port_sector_inside(RvR_port_map *map, int16_t sector, RvR_fix16 x, RvR_fix16 y)
{
   if(sector<0||sector>=map->sector_count)
      return 0;

   RvR_port_sector *sec = &map->sectors[sector];
   RvR_port_wall *wall = &map->walls[sec->wall_first];

   int64_t crossed = 0;
   for(int i = 0;i<sec->wall_count;wall++,i++)
   {
      int64_t x0 = wall->x-x;
      int64_t y0 = wall->y-y;
      int64_t x1 = map->walls[wall->p2].x-x;
      int64_t y1 = map->walls[wall->p2].y-y;

      if((y0^y1)>=0)
         continue;

      if((x0^x1)>=0)
         crossed^=x1;
      else
         //Originally: (x1-x0)*(0-y0)-(y1-y0)*(0-x0)
         crossed^=(x0*y1-x1*y0)^y1;
   }

   return crossed<0;
}

int16_t RvR_port_sector_update(RvR_port_map *map, int16_t sector_last, RvR_fix16 x, RvR_fix16 y)
{
   if(sector_last>=0&&sector_last<map->sector_count)
   {
      //Check if still in same sector
      if(RvR_port_sector_inside(map,sector_last,x,y))
         return sector_last;

      //Check adjacent sectors
      //TODO: higher depth search?
      for(int i = 0;i<map->sectors[sector_last].wall_count;i++)
      {
         int16_t portal = map->walls[map->sectors[sector_last].wall_first+i].portal;
         if(portal>=0&&RvR_port_sector_inside(map,portal,x,y))
            return portal;
      }
   }

   //Linear search over all sectors
   for(int i = 0;i<map->sector_count;i++)
      if(RvR_port_sector_inside(map,i,x,y))
         return i;

   //Not found, pretend we are still in the same sector
   return sector_last;
}

int16_t RvR_port_sector_new(RvR_port_map *map, RvR_fix16 x, RvR_fix16 y)
{
   int16_t sector = map->sector_count++;
   map->sectors = RvR_realloc(map->sectors,sizeof(*map->sectors)*map->sector_count,"Map sectors grow");
   map->sectors[sector].wall_count = 1;
   map->sectors[sector].wall_first = map->wall_count;
   map->wall_count++;
   map->walls = RvR_realloc(map->walls,sizeof(*map->walls)*map->wall_count,"Map wall grow");
   map->walls[map->sectors[sector].wall_first].x = x;
   map->walls[map->sectors[sector].wall_first].y = y;
   map->walls[map->sectors[sector].wall_first].p2 = -1;
   map->walls[map->sectors[sector].wall_first].portal = -1;
   map->walls[map->sectors[sector].wall_first].join = -1;

   return sector;
}

void RvR_port_sector_fix_winding(RvR_port_map *map, int16_t sector)
{
   int wall = 0;
   RvR_port_sector *s = map->sectors+sector;
   while(wall<map->sectors[sector].wall_count)
   {
      int64_t sum = 0;
      int first = wall;

      for(;wall==0||map->walls[map->sectors[sector].wall_first+wall-1].p2==map->sectors[sector].wall_first+wall;wall++)
      {
         int64_t x0 = map->walls[s->wall_first+wall].x;
         int64_t y0 = map->walls[s->wall_first+wall].y;
         int64_t x1 = map->walls[map->walls[s->wall_first+wall].p2].x;
         int64_t y1 = map->walls[map->walls[s->wall_first+wall].p2].y;
         sum+=(x1-x0)*(y0+y1);
      }

      if((first==0&&sum>0)||(first!=0&&sum<0))
      {
         //Need reversing
         for(int j = 0;j<(wall-first)/2;j++)
         {
            RvR_port_wall tmp = map->walls[s->wall_first+first+j];
            int pt20 = tmp.p2;
            int pt21 = map->walls[s->wall_first+wall-j-1].p2;
            map->walls[s->wall_first+first+j] = map->walls[s->wall_first+wall-j-1];

            map->walls[s->wall_first+wall-j-1] = tmp;
            map->walls[s->wall_first+first+j].p2 = pt20;
            map->walls[s->wall_first+wall-j-1].p2 = pt21;
         }
      }
      wall++;
   }
}

int16_t RvR_port_sector_make_inner(RvR_port_map *map,int16_t wall)
{
   int16_t sector = RvR_port_wall_sector(map,wall);
   int16_t first = RvR_port_wall_first(map,wall);

   int16_t sn = RvR_port_sector_new(map,map->walls[first].x,map->walls[first].y);
   map->sectors[sn].floor = map->sectors[sector].floor;
   map->sectors[sn].ceiling = map->sectors[sector].ceiling;
   map->walls[map->sectors[sn].wall_first].join = first;
   map->walls[first].join = map->sectors[sn].wall_first;
   map->walls[map->sectors[sn].wall_first].portal = sector;
   map->walls[first].portal = sn;

   int16_t w = map->walls[first].p2;
   while(w!=first)
   {
      int16_t wn = RvR_port_wall_append(map,sn,map->walls[w].x,map->walls[w].y);
      map->walls[wn].join = w;
      map->walls[w].join = wn;
      map->walls[wn].portal = sector;
      map->walls[w].portal = sn;
      w = map->walls[w].p2;
   }
   RvR_port_wall_append(map,sn,map->walls[first].x,map->walls[first].y);

   return sn;
}
//-------------------------------------
