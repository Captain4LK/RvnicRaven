/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _CAMERA_H_

#define _CAMERA_H_

#include "world_defs.h"

typedef struct
{
   //Isometric coordinates
   int x;
   int y;
   int z;

   uint8_t rotation;
}Camera;

void camera_rotate(const Camera *c, const Area *a, const int *ix, const int *iy, int *ox, int *oy);
void camera_rotate_inv(const Camera *c, const Area *a, const int *ix, const int *iy, int *ox, int *oy);

#endif
