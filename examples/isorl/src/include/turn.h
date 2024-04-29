/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _TURN_H_

#define _TURN_H_

void turn_start(World *w, Area *a);
void turns_do_until(World *w, Area *a, Entity *until);
void turn_heap_push(Entity *e);
Entity *turn_heap_max(void);
Entity *turn_heap_peek_max(void);

#endif
