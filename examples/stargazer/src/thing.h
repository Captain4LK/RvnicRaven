/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _THING_H_

#define _THING_H_

typedef enum
{
   THING_CARD, THING_ENTITY,
}Thing_type;

typedef struct
{
   Thing_type type;

   union
   {
      Card card;
      Entity entity;
   }as;
}Thing;

#endif
