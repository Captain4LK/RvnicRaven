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
#include "book.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Book **books = NULL;
//-------------------------------------

//Function prototypes
static void book_load(uint16_t id);
//-------------------------------------

//Function implementations

Book *book_get(uint16_t id)
{
   if(id>=2048)
      return book_get(0);
   if(books==NULL||books[id]==NULL)
      book_load(id);

   return books[id];
}

static void book_load(uint16_t id)
{
   if(id>=2048)
      book_load(0);

   if(books==NULL)
   {
      books = RvR_malloc(sizeof(*books)*2048,"Book ptr array");
      memset(books,0,sizeof(*books)*2048);
   }

   char tmp[64];
   snprintf(tmp,64,"BOK%05d",id);

   size_t size_in;
   uint8_t *mem_pak = NULL;
   mem_pak = RvR_lump_get(tmp, &size_in);
   RvR_error_check(size_in!=0, "book_get", "BOK%05d not found\n", id);
   RvR_mem_tag_set(mem_pak, RVR_MALLOC_STATIC);

   RvR_rw rw_pak = {0};
   RvR_rw_init_const_mem(&rw_pak,mem_pak,size_in);
   books[id] = RvR_malloc(sizeof(*books[id]),"Book");
   RvR_rw_read(&rw_pak,books[id]->title,1,64);
   RvR_rw_read(&rw_pak,books[id]->author,1,64);
   RvR_rw_read(&rw_pak,books[id]->date,1,16);
   books[id]->words = RvR_rw_read_u32(&rw_pak);
   books[id]->bcase = RvR_rw_read_u8(&rw_pak);
   books[id]->shelf = RvR_rw_read_u8(&rw_pak);
   books[id]->slot = RvR_rw_read_u8(&rw_pak);
   RvR_rw_close(&rw_pak);

   RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);
   RvR_mem_tag_set(books[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(books[id], (void **)&books[id]);

   return;

RvR_err:
   if(mem_pak!=NULL)
      RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   books[id] = RvR_malloc(sizeof(*books[id]),"Book");
   strcpy(books[id]->title,"???");
   strcpy(books[id]->author,"???");
   strcpy(books[id]->date,"???");
   books[id]->words = 2048;
   books[id]->bcase = 0;
   books[id]->shelf = 0;
   books[id]->slot = 0;

   RvR_mem_tag_set(books[id], RVR_MALLOC_CACHE);
   RvR_mem_usr_set(books[id], (void **)&books[id]);

   return;
}
//-------------------------------------
