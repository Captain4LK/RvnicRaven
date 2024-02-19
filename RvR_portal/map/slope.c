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
   //TODO(Captain4LK): increase accuracy
   RvR_fix22 rx = RvR_fix22_cos(map->sectors[sector].slope_floor-1024);
   RvR_fix22 nx = 0;
   RvR_fix22 ny = 0;
   RvR_fix22 nz = -RvR_fix22_sin(map->sectors[sector].slope_floor-1024);

   RvR_fix22 x0;
   RvR_fix22 y0;
   RvR_fix22 x1;
   RvR_fix22 y1;
   RvR_port_wall *w0 = map->walls+map->sectors[sector].wall_first;
   RvR_port_wall *w1 = map->walls+w0->p2;
   x0 = w0->x;
   y0 = w0->y;
   x1 = w1->x;
   y1 = w1->y;

   //TODO(Captain4LK): does this work properly?
   //TODO(Captain4LK): And can we multiply rx by (x1-x0)^2 to get rid of sqrt?
   RvR_fix22 len = RvR_fix22_sqrt(RvR_fix22_mul(x1-x0,x1-x0)+RvR_fix22_mul(y1-y0,y1-y0));
   nx = RvR_fix22_mul(rx,x1-x0);
   ny = RvR_fix22_mul(rx,-(y1-y0));
   nz = RvR_fix22_mul(len,nz);
   //RvR_fix22 len = RvR_fix22_sqrt(RvR_fix22_mul(nx,nx)+RvR_fix22_mul(ny,ny)+RvR_fix22_mul(nz,nz));
   //nx = RvR_fix22_div(nx,RvR_non_zero(len));
   //ny = RvR_fix22_div(ny,RvR_non_zero(len));
   //nz = RvR_fix22_div(nz,RvR_non_zero(len));

   slope->x = nx;
   slope->y = ny;
   slope->z = nz;
   slope->d = -(RvR_fix22_mul(nx,x0)+RvR_fix22_mul(ny,y0)+RvR_fix22_mul(nz,map->sectors[sector].floor));
   //printf("%d %d %d %d; %d\n",slope->x,slope->y,slope->z,slope->d,map->sectors[sector].slope_floor);

   /*len = RvR_fix22_sqrt(RvR_fix22_mul(nx,nx)+RvR_fix22_mul(ny,ny)+RvR_fix22_mul(nz,nz));
   nx = RvR_fix22_div(nx,RvR_non_zero(len));
   ny = RvR_fix22_div(ny,RvR_non_zero(len));
   nz = RvR_fix22_div(nz,RvR_non_zero(len));
   slope->x = nx;
   slope->y = ny;
   slope->z = nz;
   slope->d = -(RvR_fix22_mul(nx,x0)+RvR_fix22_mul(ny,y0)+RvR_fix22_mul(nz,map->sectors[sector].floor));
   printf("%d %d %d %d; %d\n",slope->x,slope->y,slope->z,slope->d,map->sectors[sector].slope_floor);*/
}

void RvR_port_slope_from_floor_vs(const RvR_port_map *map, const RvR_port_cam *cam, uint16_t sector, RvR_port_slope *slope)
{
   RvR_fix22 sin = RvR_fix22_sin(cam->dir);
   RvR_fix22 cos = RvR_fix22_cos(cam->dir);

   //TODO(Captain4LK): increase accuracy
   RvR_fix22 rx = RvR_fix22_cos(map->sectors[sector].slope_floor-1024);
   RvR_fix22 nx = 0;
   RvR_fix22 ny = 0;
   RvR_fix22 nz = -RvR_fix22_sin(map->sectors[sector].slope_floor-1024);

   RvR_fix22 x0;
   RvR_fix22 y0;
   RvR_fix22 x1;
   RvR_fix22 y1;
   RvR_port_wall *w0 = map->walls+map->sectors[sector].wall_first;
   RvR_port_wall *w1 = map->walls+w0->p2;
   x0 = w0->x-cam->x;
   y0 = w0->y-cam->y;
   x1 = w1->x-cam->x;
   y1 = w1->y-cam->y;
   RvR_fix22 tmp = x0;
   x0 = RvR_fix22_mul(-x0,sin)+RvR_fix22_mul(y0,cos);
   y0 = RvR_fix22_mul(tmp,cos)+RvR_fix22_mul(y0,sin);
   tmp = x1;
   x1 = RvR_fix22_mul(-x1,sin)+RvR_fix22_mul(y1,cos);
   y1 = RvR_fix22_mul(tmp,cos)+RvR_fix22_mul(y1,sin);

   //TODO(Captain4LK): does this work properly?
   //TODO(Captain4LK): And can we multiply rx by (x1-x0)^2 to get rid of sqrt?
   RvR_fix22 len = RvR_fix22_sqrt(RvR_fix22_mul(x1-x0,x1-x0)+RvR_fix22_mul(y1-y0,y1-y0));
   nx = RvR_fix22_mul(rx,x1-x0);
   ny = RvR_fix22_mul(rx,-(y1-y0));
   nz = RvR_fix22_mul(len,nz);

   slope->x = nx;
   slope->y = ny;
   slope->z = nz;
   slope->d = -(RvR_fix22_mul(nx,x0)+RvR_fix22_mul(ny,y0)+RvR_fix22_mul(nz,map->sectors[sector].floor-cam->z));
}

void RvR_port_slope_from_ceiling(const RvR_port_map *map, uint16_t sector, RvR_port_slope *slope)
{
}

RvR_fix22 RvR_port_slope_height_at(const RvR_port_slope *slope, RvR_fix22 x, RvR_fix22 y)
{
   int64_t num = -(slope->x*x+slope->y*y+slope->d);
   return (RvR_fix22)(num/RvR_non_zero(slope->z));
}
//-------------------------------------
