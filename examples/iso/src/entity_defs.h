/*
RvnicRaven - iso roguelike 

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ENTITY_TYPE_H_

#define _ENTITY_TYPE_H_

typedef struct Entity Entity;

typedef enum
{
   ACTION_INVALID = -1,
   ACTION_WAIT = 0,
   ACTION_MOVE = 1,
}Action_id;

typedef struct
{
   Action_id id;
   int status;
}Action;

struct Entity
{
   int16_t x;
   int16_t y;
   int16_t z;
   uint16_t tex;

   int speed;

   uint64_t id;
   int removed;

   Action action;

   Entity *next;
   Entity **prev_next;

   Entity *g_next;
   Entity **g_prev_next;
};

typedef struct
{
   uint64_t id;
   Entity *index;
}Gendex;

#endif
