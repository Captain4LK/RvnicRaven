/*
RvnicRaven - palette and color luts

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdint.h>
#include <stddef.h>
//-------------------------------------

//Internal includes
#include "RvR/RvR_hash.h"
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

#define RVR_HASH_FNV32_PRIME (UINT32_C(0x01000193))
#define RVR_HASH_FNV64_PRIME (UINT64_C(0x100000001b3))

uint64_t RvR_fnv64a(const char *str)
{
   if(str==NULL)
      return RVR_HASH_FNV64_INIT;

   return RvR_fnv64a_str(str, RVR_HASH_FNV64_INIT);
}

uint64_t RvR_fnv64a_str(const char *str, uint64_t hval)
{
   if(str==NULL)
      return hval;

   unsigned char *s = (unsigned char *)str;
   while(*s)
   {
      hval ^= (uint64_t)*s++;
      hval *= RVR_HASH_FNV64_PRIME;
   }

   return hval;
}

uint64_t RvR_fnv64a_buf(const void *buf, size_t len, uint64_t hval)
{
   if(buf==NULL)
      return hval;

   unsigned char *bs = (unsigned char *)buf;
   unsigned char *be = bs + len;
   while(bs<be)
   {
      hval ^= (uint64_t)*bs++;
      hval *= RVR_HASH_FNV64_PRIME;
   }

   return hval;
}

uint32_t RvR_fnv32a(const char *str)
{
   if(str==NULL)
      return RVR_HASH_FNV32_INIT;

   return RvR_fnv32a_str(str, RVR_HASH_FNV32_INIT);
}

uint32_t RvR_fnv32a_str(const char *str, uint32_t hval)
{
   if(str==NULL)
      return hval;

   unsigned char *s = (unsigned char *)str;
   while(*s)
   {
      hval ^= (uint32_t)*s++;
      hval *= RVR_HASH_FNV32_PRIME;
   }

   return hval;
}

uint32_t RvR_fnv32a_buf(const void *buf, size_t len, uint32_t hval)
{
   if(buf==NULL)
      return hval;

   unsigned char *bs = (unsigned char *)buf;
   unsigned char *be = bs + len;
   while(bs<be)
   {
      hval ^= (uint32_t)*bs++;
      hval *= RVR_HASH_FNV32_PRIME;
   }

   return hval;
}
//-------------------------------------
