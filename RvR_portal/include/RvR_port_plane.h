/*
RvnicRaven-portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PORT_PLANE_H_

#define _RVR_PORT_PLANE_H_

typedef struct port_plane port_plane;
struct port_plane
{
   int32_t min;
   int32_t max;
   uint16_t sector;
   uint8_t where; //0 --> ceiling; 1 --> floor
   uint16_t start[RVR_XRES_MAX+2];
   uint16_t end[RVR_XRES_MAX+2];

   port_plane *next;
};

void port_planes_draw();
void port_planes_begin();
void port_plane_add(int16_t sector, uint8_t where, int x, int y0, int y1);
void port_plane_free(port_plane *pl);

#endif
