/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "bookcase.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Bookcase bookcases[BOOKCASE_COUNT];
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

uint16_t bookcase_insert(uint8_t id, uint8_t shelf, uint8_t slot, uint16_t book)
{
   if(id>=BOOKCASE_COUNT||shelf>=3||slot>=14)
      return BOOK_INVALID;

   uint16_t prev = bookcases[id].books[shelf*14+slot];
   bookcases[id].books[shelf*14+slot] = book;
   bookcase_redraw(id);

   return prev;
}

uint16_t bookcase_remove(uint8_t id, uint8_t shelf, uint8_t slot)
{
   uint16_t book = bookcase_insert(id,shelf,slot,BOOK_INVALID);
   bookcase_redraw(id);
   return book;
}

uint16_t bookcase_at(uint8_t id, uint8_t shelf, uint8_t slot)
{
   if(id>=BOOKCASE_COUNT||shelf>=3||slot>=14)
      return BOOK_INVALID;

   return bookcases[id].books[shelf*14+slot];
}

void bookcase_clear(uint8_t id)
{
   if(id>=BOOKCASE_COUNT)
      return;

   for(uint8_t i = 0;i<3;i++)
   {
      for(uint8_t j = 0;j<14;j++)
      {
         bookcase_remove(id,i,j);
      }
   }

   bookcase_redraw(id);
}

uint16_t bookcase_texture(uint8_t id)
{
   if(id>=BOOKCASE_COUNT)
      return 0;

   return UINT16_MAX-BOOKCASE_COUNT+id+1;
}

void bookcase_redraw(uint8_t id)
{
   if(id>=BOOKCASE_COUNT)
      return;
}
//-------------------------------------
