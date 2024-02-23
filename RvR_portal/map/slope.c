/*
RvnicRaven-portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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

void RvR_port_slope_from_floor(const RvR_port_map *map, uint16_t sector, RvR_port_slope *slope)
{
   RvR_fix22 sl = map->sectors[sector].slope_floor;
   RvR_port_wall *w0 = map->walls+map->sectors[sector].wall_first;
   RvR_port_wall *w1 = map->walls+w0->p2;
   RvR_fix22 x0 = w0->x;
   RvR_fix22 y0 = w0->y;
   RvR_fix22 x1 = w1->x;
   RvR_fix22 y1 = w1->y;

   RvR_fix22 px = x1-x0;
   RvR_fix22 py = y1-y0;
   RvR_fix22 pz = 0;

   RvR_fix22 dirx = RvR_fix22_cos(sl);
   RvR_fix22 diry = -RvR_fix22_sin(sl);
   
   RvR_fix22 qx = -(y1-y0);
   RvR_fix22 qy = x1-x0;
   //TODO(Captain4LK): can we somehow get rid of the sqrt here?
   RvR_fix22 qz = RvR_fix22_mul(-RvR_fix22_sin(sl),RvR_fix22_sqrt(RvR_fix22_mul(px,px)+RvR_fix22_mul(py,py)));

   RvR_fix22 nx = RvR_fix22_mul(py,qz)-RvR_fix22_mul(pz,qy);
   RvR_fix22 ny = RvR_fix22_mul(pz,qx)-RvR_fix22_mul(px,qz);
   RvR_fix22 nz = RvR_fix22_mul(px,qy)-RvR_fix22_mul(py,qx);

   slope->x = nx;
   slope->y = ny;
   slope->z = nz;
   //NOTE(Captain4LK): we store the plane as normal + origin instead of normal + d
   //since it caused problems with overflows
   slope->ox = x0;
   slope->oy = y0;
   slope->oz = map->sectors[sector].floor;
}

void RvR_port_slope_from_ceiling(const RvR_port_map *map, uint16_t sector, RvR_port_slope *slope)
{
   RvR_fix22 sl = map->sectors[sector].slope_ceiling;
   RvR_port_wall *w0 = map->walls+map->sectors[sector].wall_first;
   RvR_port_wall *w1 = map->walls+w0->p2;
   RvR_fix22 x0 = w0->x;
   RvR_fix22 y0 = w0->y;
   RvR_fix22 x1 = w1->x;
   RvR_fix22 y1 = w1->y;

   RvR_fix22 px = x1-x0;
   RvR_fix22 py = y1-y0;
   RvR_fix22 pz = 0;
   RvR_fix22 qx = -(y1-y0);
   RvR_fix22 qy = x1-x0;
   RvR_fix22 qz = RvR_fix22_mul(-RvR_fix22_sin(sl),RvR_fix22_sqrt(RvR_fix22_mul(px,px)+RvR_fix22_mul(py,py)));

   RvR_fix22 nx = RvR_fix22_mul(py,qz)-RvR_fix22_mul(pz,qy);
   RvR_fix22 ny = RvR_fix22_mul(pz,qx)-RvR_fix22_mul(px,qz);
   RvR_fix22 nz = RvR_fix22_mul(px,qy)-RvR_fix22_mul(py,qx);

   slope->x = nx;
   slope->y = ny;
   slope->z = nz;
   slope->ox = x0;
   slope->oy = y0;
   slope->oz = map->sectors[sector].ceiling;
}

RvR_fix22 RvR_port_slope_height_at(const RvR_port_slope *slope, RvR_fix22 x, RvR_fix22 y)
{
   int64_t num = (int64_t)slope->x*x+(int64_t)slope->y*y-(int64_t)slope->x*slope->ox-(int64_t)slope->y*slope->oy-(int64_t)slope->z*slope->oz;
   return (RvR_fix22)(-num/RvR_non_zero(slope->z));
}
//-------------------------------------
