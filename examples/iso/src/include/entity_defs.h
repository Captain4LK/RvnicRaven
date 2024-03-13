/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ENTITY_TYPE_H_

#define _ENTITY_TYPE_H_

#include "action_defs.h"
#include "defs.h"
#include "body.h"
#include "point.h"

typedef struct Entity Entity;
typedef struct Group Group;

typedef enum
{
   AI_INVALID = 0,
   AI_PLAYER,
}AI_type;

typedef enum
{
   ENTITY_SPEC_HUMAN = 0,

   ENTITY_SPEC_CAT = 1,
   ENTITY_SPEC_DOG = 2,
}Entity_species_index;

typedef struct
{
   uint64_t flags;
}Entity_species;

typedef enum
{
   ENTITY_GEN_MALE = 0,
   ENTITY_GEN_FEMALE = 1,
}Entity_gender;

typedef enum
{
   ENTITY_PROF_NONE = 0,
}Entity_profession;

typedef struct
{
   Entity_species_index species;
   Entity_gender gender;

   //For intelligent species
   Entity_profession profession;
}Entity_identifier;

struct Entity
{
   Point pos;
   uint16_t sprite;

   int speed;
   int action_points;

   //>UINT64_MAX/2 --> non-historic
   uint64_t id;
   //int removed;

   //
   int fatigue;
   int hunger;
   //Timers
   int fatigue_next;
   int hunger_next;

   Entity_identifier ident;
   Body body;

   Action action;

   AI_type ai_type;

   Entity *next;
   Entity **prev_next;

   Entity *g_next;
   Entity **g_prev_next;
};

typedef struct
{
   Entity_identifier ident;

   uint16_t mx;
   uint16_t my;
   uint16_t ax;
   uint16_t ay;

   char name[64];

   uint64_t id;

   //Needs saving
   uint8_t modified;
}Entity_documented;

//single person, group, legions etc.
/*struct Group
{
};*/

typedef struct
{
   uint64_t id;
   Entity *index;
}Entity_index;

#endif
