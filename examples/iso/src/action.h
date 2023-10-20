/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ACTION_H_

#define _ACTION_H_

#include "world_defs.h"
#include "action_defs.h"
#include "entity_defs.h"
#include "point.h"

int action_do(World *w, Area *a, Entity *e);

void action_free(Entity *e);

void action_set_wait(Entity *e, uint32_t time);
void action_set_move(Entity *e, uint8_t dir);
void action_set_ascend(Entity *e);
void action_set_descend(Entity *e);
void action_set_attack(Entity *e, uint8_t dir);
void action_set_path(Area *a, Entity *e, Point goal);

void action_interrupt(Entity *e);

#endif
