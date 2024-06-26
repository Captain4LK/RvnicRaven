/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _PLAYER_H_

#define _PLAYER_H_

#include "camera.h"
#include "entity_defs.h"
#include "world_defs.h"

typedef struct
{
   Entity *e;
   Camera cam;

   uint64_t id;
   int menu_state;
}Player;

extern Player player;

void player_new(World *w, Area *a);
void player_add(World *w, Area *a);
int player_update();
void player_menu_draw();

#endif
