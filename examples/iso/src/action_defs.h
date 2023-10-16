/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ACTION_DEFS_H_

#define _ACTION_DEFS_H_

typedef enum
{
   ACTION_INVALID = -1,
   ACTION_WAIT = 0,
   ACTION_MOVE = 1,

   ACTION_ASCEND = 2,
   ACTION_DESCEND = 3,
   
   ACTION_ATTACK = 4,

   ACTION_PATH = 5,
}Action_id;

typedef enum
{
   ACTION_IN_PROGRESS = 0,

   ACTION_FINISHED = 1,
   ACTION_LEFT_MAP,
}Action_status;

typedef struct
{
   uint32_t time;
}AWait;

typedef struct
{
   uint8_t dir;
}AMove;

typedef struct
{
   uint8_t dir;
}AAttack;

typedef struct
{
   uint8_t *path;
   uint32_t pos;
   uint32_t len;
   int16_t x;
   int16_t y;
   int16_t z;
}APath;

typedef struct
{
   Action_id id;
   int status;

   int remaining;
   int interrupt;
   int can_interrupt;

   union
   {
      AWait wait;
      AMove move;
      AAttack attack;
      APath path;
   }as;
}Action;

#endif
