/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ENTITY_H_

#define _ENTITY_H_

#include "gamestate.h"

typedef struct
{
   uint64_t generation;
   Entity *index;
}Gendex;

inline Gendex gendex_get(Entity *e)
{
   if(e==NULL)
      return (Gendex){.generation = 0, .index = NULL};
   return (Gendex){.generation = e->generation, .index = e};
}

inline Entity *gendex_try(Gendex gen)
{
   if(gen.index==NULL)
      return NULL;

   if(gen.generation==gen.index->generation)
      return gen.index;

   return NULL;
}

Entity *entity_new();
void entity_free(Entity *e);
void entity_add(Gamestate *state, Entity *e);
void entity_remove(Entity *e);

#endif
