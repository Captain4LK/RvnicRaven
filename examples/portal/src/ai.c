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
#include "entity.h"
#include "ai.h"

#include "ai/ai_none.h"
#include "ai/ai_book.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void ai_run(Entity *e)
{
   if(e==NULL)
      return;

   switch(e->ai_type)
   {
   case AI_NONE: ai_none_run(e); break;
   case AI_BOOK: ai_book_run(e); break;
   }
}

void ai_free(Entity *e)
{
   if(e==NULL)
      return;

   switch(e->ai_type)
   {
   case AI_NONE: ai_none_free(e); break;
   case AI_BOOK: ai_book_free(e); break;
   }
}

void ai_init(Entity *e, uint32_t ai_type, const uint32_t args[4])
{
   if(e==NULL)
      return;

   e->ai_type = ai_type;

   switch(ai_type)
   {
   case AI_NONE: ai_none_init(e,args); break;
   case AI_BOOK: ai_book_init(e,args); break;
   }
}

void ai_on_use(Entity *e, Entity *trigger)
{
   if(e==NULL)
      return;

   switch(e->ai_type)
   {
   case AI_NONE: ai_none_on_use(e,trigger); break;
   case AI_BOOK: ai_book_on_use(e,trigger); break;
   }
}
//-------------------------------------
