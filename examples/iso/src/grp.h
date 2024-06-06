/*
RvnicRaven - iso

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _GRP_H_

#define _GRP_H_

typedef struct
{
   //width = 32
   //heigh = 32
   uint32_t row_offsets[32];
   uint8_t data[];
}GRP_floor;

typedef struct
{
   //width = 32
   //heigh = 32
   uint32_t row_offsets[32];
   uint8_t data[];
}GRP_block;

typedef struct
{
   //width = 32
   //height = 32*4
   uint32_t row_offsets[32 * 4];
   uint8_t data[];
}GRP_wall;

GRP_floor *grp_floor_get(uint16_t id);
GRP_block *grp_block_get(uint16_t id);
GRP_wall  *grp_wall_get(uint16_t id);

#endif
