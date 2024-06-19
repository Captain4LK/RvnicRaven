/*
RvnicRaven - general math utilities

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
//-------------------------------------

//Internal includes
#include "RvR/RvR_math.h"
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

int RvR_clz8_gen(uint8_t a)
{
   int n = 1;
   if(a==0) return 8;
   if((a>>4)==0) { n+=4; a<<=4; }
   if((a>>6)==0) { n+=2; a<<=2; }
   n = n-(int)(a>>7);

   return n;
}

int RvR_clz16_gen(uint16_t a)
{
   int n = 1;
   if(a==0) return 16;
   if((a>>8)==0)  { n+=8; a<<=8; }
   if((a>>12)==0) { n+=4; a<<=4; }
   if((a>>14)==0) { n+=2; a<<=2; }
   n = n-(int)(a>>15);

   return n;
}

int RvR_clz32_gen(uint32_t a)
{
   int n = 1;
   if(a==0) return 32;
   if((a>>16)==0) { n+=16; a<<=16; }
   if((a>>24)==0) { n+=8;  a<<=8; }
   if((a>>28)==0) { n+=4;  a<<=4; }
   if((a>>30)==0) { n+=2;  a<<=2; }
   n = n-(int)(a>>31);

   return n;
}

int RvR_clz64_gen(uint64_t a)
{
   int n = 1;
   if(a==0) return 64;
   if((a>>32)==0) { n+=32; a<<=32; }
   if((a>>48)==0) { n+=16; a<<=16; }
   if((a>>56)==0) { n+=8; a<<=8; }
   if((a>>60)==0) { n+=4; a<<=4; }
   if((a>>62)==0) { n+=2; a<<=2; }
   n = n-(int)(a>>63);

   return n;
}
//-------------------------------------
