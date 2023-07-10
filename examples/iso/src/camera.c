/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "camera.h"
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

void camera_rotate(const Camera *c, const Area *a, const int *ix, const int *iy, int *ox, int *oy)
{
   if(c==NULL||a==NULL)
      return;

   int x = *ix;
   int y = *iy;

   switch(c->rotation)
   {
   case 0: *ox = x; *oy = y; break;
   case 1: *ox = a->dimy*32-1-y; *oy = x; break;
   case 2: *ox = a->dimx*32-1-x; *oy = a->dimy*32-1-y; break;
   case 3: *ox = y; *oy = a->dimx*32-1-x; break;
   }
}

void camera_rotate_inv(const Camera *c, const Area *a, const int *ix, const int *iy, int *ox, int *oy)
{
   if(c==NULL||a==NULL)
      return;

   int x = *ix;
   int y = *iy;

   switch(c->rotation)
   {
   case 0: *ox = x; *oy = y; break;
   case 1: *ox = y; *oy = a->dimx*32-1-x; break;
   case 2: *ox = a->dimx*32-1-x; *oy = a->dimy*32-1-y; break;
   case 3: *ox = a->dimy*32-1-y; *oy = x; break;
   }
}
//-------------------------------------
