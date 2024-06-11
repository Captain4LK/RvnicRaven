/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _BOOKSHELF_H_

#define _BOOKSHELF_H_

//Probably about 24 bookcases --> 1008 books?
typedef struct
{
   //3 shelves, 14 slots each
   uint16_t books[42];
}Bookcase;

uint16_t bookcase_insert(uint8_t id, uint8_t shelf, uint8_t slot, uint16_t book); //returns book previously in slot
uint16_t bookcase_remove(uint8_t id, uint8_t shelf, uint8_t slot);
uint16_t bookcase_at(uint8_t id, uint8_t shelf, uint8_t slot);
void bookcase_clear(uint8_t id);
uint16_t bookcase_texture(uint8_t id);
void bookcase_redraw(uint8_t id);

#endif
