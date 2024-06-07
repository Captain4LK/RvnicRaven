/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _BOOK_H_

#define _BOOK_H_

typedef struct
{
   char title[64];
   char author[64];
   char date[16];
   //char quote[512];
   uint32_t words;

   //For identifier, not forced
   uint8_t bcase;
   uint8_t shelf;
   uint8_t slot;
}Book;

Book *book_get(uint16_t id);

#endif
