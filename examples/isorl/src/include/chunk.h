/*
RvnicRaven - iso roguelike

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _CHUNK_H_

#define _CHUNK_H_

Point chunk_pos_to_area(const Area *a, const Chunk *c, Point pos);
Chunk *chunk_get(World *w, int x, int y, int z);
void chunk_unload(World *w, Chunk *c); //frees chunk

#endif
