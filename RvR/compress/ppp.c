/*
RvnicRaven - data (de-)compression: ppp

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvR/RvR_rw.h"
#include "RvR/RvR_compress.h"
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

void RvR_ppp_compress_init(RvR_ppp_ccontext *c, RvR_rw *dst)
{
   memset(c->guess_table, 0, sizeof(c->guess_table));
   c->dst = dst;

   c->hash = 0;
   c->flag = 0;
   c->byte = 0;
}

void RvR_ppp_compress_push(RvR_ppp_ccontext *c, uint8_t byte)
{
   //Incorrect guess --> update hash table and add byte to buffer
   if(c->guess_table[c->hash]!=byte)
   {
      c->guess_table[c->hash] = byte;
      c->byte_buf[c->byte] = byte;
   }
   //Correct guess --> set flag
   else
   {
      c->flag |= (1 << c->byte);
   }

   //Update hash
   c->hash = ((c->hash * 160) ^ (byte)) & ((1 << 18) - 1);
   c->byte++;

   //Every eight bytes, write the flag byte
   //followed by all incorrect guess bytes
   if(c->byte==8)
   {
      RvR_rw_write_u8(c->dst, c->flag);
      for(int i = 0; i<8; i++)
         if(!(c->flag & (1 << i)))
            RvR_rw_write_u8(c->dst, c->byte_buf[i]);

      c->byte = 0;
      c->flag = 0;
   }
}

void RvR_ppp_compress_flush(RvR_ppp_ccontext *c, uint8_t pad)
{
   //write pad bytes until the buffer is flushed
   while(c->byte!=0)
      RvR_ppp_compress_push(c, pad);
}

void RvR_ppp_decompress_init(RvR_ppp_dcontext *c, RvR_rw *src)
{
   memset(c->guess_table, 0, sizeof(c->guess_table));
   c->src = src;

   c->hash = 0;
   c->flag = 0;
   c->byte = 0;
}

int16_t RvR_ppp_decompress_pop(RvR_ppp_dcontext *c)
{
   //Every eight bytes read, update the flag byte
   if(c->byte==0)
   {
      if(RvR_rw_eof(c->src))
         return -1;
      c->flag = RvR_rw_read_u8(c->src);
   }

   //If the flag byte is set, we can read the byte from the hash table,
   //otherwise we need to read it from the input stream
   uint8_t byte;
   if(c->flag & (1 << c->byte))
      byte = c->guess_table[c->hash];
   else
      c->guess_table[c->hash] = byte = RvR_rw_read_u8(c->src);

   //Update hash
   c->hash = ((c->hash * 160) ^ (byte)) & ((1 << 18) - 1);

   c->byte++;
   if(c->byte==8)
      c->byte = 0;

   return byte;
}
//-------------------------------------
