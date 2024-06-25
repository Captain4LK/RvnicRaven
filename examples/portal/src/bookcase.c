/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "bookcase.h"
#include "book.h"
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

   return prev;
}

uint16_t bookcase_remove(uint8_t id, uint8_t shelf, uint8_t slot)
{
   uint16_t book = bookcase_insert(id,shelf,slot,BOOK_INVALID);
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
}

uint16_t bookcase_texture(uint8_t id)
{
   if(id>=BOOKCASE_COUNT)
      return 0;

   return UINT16_MAX-id;
}

void bookcase_redraw(uint8_t id)
{
   if(id>=BOOKCASE_COUNT)
      return;

   RvR_texture_create(bookcase_texture(id),64,128);
   RvR_texture *new = RvR_texture_get(bookcase_texture(id));
   RvR_texture *src = RvR_texture_get(1);
   memcpy(new->data,src->data,sizeof(*src->data)*64*128);

   for(int y = 0;y<3;y++)
   {
      for(int x = 0;x<14;x++)
      {
         uint16_t b = bookcase_at(id,y,x);
         if(b==BOOK_INVALID)
            continue;

         Book *book = book_get(b);
         if(book==NULL)
            continue;

         for(int ty = 0;ty<16;ty++)
         {
            for(int tx = 0;tx<4;tx++)
            {
               new->data[(x*4+4+tx)*128+y*24+8+ty] = book->tex[ty*4+tx];
            }
         }
      }
   }
}
//-------------------------------------
