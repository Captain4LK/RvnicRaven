/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "point.h"
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

Point point_add_dir(Point p, uint8_t dir)
{
   const Point dirs[26] = { { 1, 0 ,0}, { 0, 1 ,0}, { -1, 0 ,0}, { 0, -1 ,0}, { -1, 1 ,0}, { -1, -1 ,0}, { 1, -1 ,0}, { 1, 1 ,0}, {0,0,1}, {0,0,-1},
    { 1, 0 ,-1}, { 0, 1 ,-1}, { -1, 0 ,-1}, { 0, -1 ,-1}, { -1, 1 ,-1}, { -1, -1 ,-1}, { 1, -1 ,-1}, { 1, 1 ,-1},
    { 1, 0 ,1}, { 0, 1 ,1}, { -1, 0 ,1}, { 0, -1 ,1}, { -1, 1 ,1}, { -1, -1 ,1}, { 1, -1 ,1}, { 1, 1 ,1}};

   if(dir>=26)
      return p;

   p.x+=dirs[dir].x;
   p.y+=dirs[dir].y;
   p.z+=dirs[dir].z;

   return p;
}

Point point_sub_dir(Point p, uint8_t dir)
{
   const Point dirs[26] = { { 1, 0 ,0}, { 0, 1 ,0}, { -1, 0 ,0}, { 0, -1 ,0}, { -1, 1 ,0}, { -1, -1 ,0}, { 1, -1 ,0}, { 1, 1 ,0}, {0,0,1}, {0,0,-1},
    { 1, 0 ,-1}, { 0, 1 ,-1}, { -1, 0 ,-1}, { 0, -1 ,-1}, { -1, 1 ,-1}, { -1, -1 ,-1}, { 1, -1 ,-1}, { 1, 1 ,-1},
    { 1, 0 ,1}, { 0, 1 ,1}, { -1, 0 ,1}, { 0, -1 ,1}, { -1, 1 ,1}, { -1, -1 ,1}, { 1, -1 ,1}, { 1, 1 ,1}};

   if(dir>=26)
      return p;

   p.x-=dirs[dir].x;
   p.y-=dirs[dir].y;
   p.z-=dirs[dir].z;

   return p;
}

int point_equal(Point a, Point b)
{
   return (a.x==b.x&&a.y==b.y&&a.z==b.z);
}

Point point(int x, int y, int z)
{
   //TODO(Captain4LK): change parameters to int16_t and handle the great flood of warnings
   return (Point){.x = (int16_t)x, .y = (int16_t)y, .z = (int16_t)z};
}

int point_adjacent(Point a, Point b)
{
   if(a.z!=b.z)
      return 0;

   return RvR_abs(a.x-b.x)<=1&&RvR_abs(a.y-b.y)<=1;
}
//-------------------------------------
