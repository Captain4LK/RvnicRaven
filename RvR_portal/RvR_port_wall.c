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
//-------------------------------------
