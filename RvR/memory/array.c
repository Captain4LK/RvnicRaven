/*
RvnicRaven - dynamic arrays

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR/RvR_log.h"
#include "RvR/RvR_malloc.h"
#include "RvR/RvR_ds.h"
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

void *RvR_array_grow_internal(void *old, size_t size, size_t grow, size_t min)
{
   size_t header_off = (sizeof(RvR_aheader) + size - 1) / size;

   if(min<4)
      min = 4;
   if(old==NULL)
   {
      //goodbye c++ lmao
      char *new = RvR_malloc(header_off * size + min * size, "RvR_array new");
      RvR_aheader *h = (RvR_aheader *)new;
      h->length = 0;
      h->size = min;

      return new + header_off * size;
   }

   RvR_aheader *h = (RvR_aheader *)(((char *)old) - header_off * size);
   if(h->size<min)
      h->size = min;
   while(h->size<h->length + grow)
      h->size *= 2;
   h = RvR_realloc(h, header_off * size + h->size * size, "RvR_array grow");

   return ((char *)h) + header_off * size;
}
//-------------------------------------
