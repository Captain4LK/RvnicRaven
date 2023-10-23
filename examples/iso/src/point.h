/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _POINT_H_

#define _POINT_H_

typedef struct
{
   int16_t x;
   int16_t y;
   int16_t z;
}Point;

Point point_add_dir(Point p, uint8_t dir);
Point point_sub_dir(Point p, uint8_t dir);
int point_equal(Point a, Point b);
Point point(int x, int y, int z);
int point_adjacent(Point a, Point b);
uint8_t point_dir(Point from, Point to);

#endif
