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
//-------------------------------------
