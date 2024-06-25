/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _ITEM_H_

#define _ITEM_H_

typedef enum
{
   ITEM_INVALID = 0,
   ITEM_BOOK = 1,
}Item_type;

typedef struct
{
   Item_type type;

   union
   {
      uint16_t book;
   }as;
}Item;

#endif
