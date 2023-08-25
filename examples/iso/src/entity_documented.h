/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ENTITY_DOCUMENTED_H_

#define _ENTITY_DOCUMENTED_H_

#include "world_defs.h"
#include "entity_defs.h"

void entity_doc_init_table(World *w);
void entity_doc_get(World *w, Entity_documented *e, uint64_t id);
void entity_doc_modify(World *w, uint64_t id, const Entity_documented *e);
//TODO(Captain4LK): function for saving individual DocEnt?
void entity_doc_save_modified(World *w);

Entity *entity_from_docent(World *w, Area *a, uint64_t id);

#endif
