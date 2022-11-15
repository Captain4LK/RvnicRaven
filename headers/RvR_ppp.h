#ifndef _RVR_PPP_H_

/*
   RvnicRaven - ppp streaming (de-)compressor

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
   To create implementation (the function definitions) add
      #define RVR_PPP_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_PPP_H_

typedef struct
{
   uint8_t guess_table[1<<18];
   uint32_t hash;
   RvR_rw *dst;

   uint8_t byte;
   uint8_t byte_buf[8];
   uint8_t flag;
}RvR_ppp_context;

void RvR_ppp_compress_init(RvR_ppp_context *c, RvR_rw *dst);
void RvR_ppp_compress_push(RvR_ppp_context *c, uint8_t byte);
void RvR_ppp_compress_flush(RvR_ppp_context *c, uint8_t pad);

#endif

#ifdef RVR_PPP_IMPLEMENTATION
#ifndef RVR_PPP_IMPLEMENTATION_ONCE
#define RVR_PPP_IMPLEMENTATION_ONCE

void RvR_ppp_compress_init(RvR_ppp_context *c, RvR_rw *dst)
{
   memset(c->guess_table,0,sizeof(c->guess_table));
   c->hash = 0;
   c->dst = dst;
   c->flag = 0;
}

void RvR_ppp_compress_push(RvR_ppp_context *c, uint8_t byte)
{
   if(c->guess[c->hash]!=byte)
   {
      c->guess_table[c->hash] = byte;
      c->byte_buf[c->byte] = byte;
   }
   else
   {
      c->flag|=(1<<c->byte);
   }

   c->hash = ((c->hash*160)^(byte))&((1<<18)-1);
   c->byte++;
   if(c->byte==8)
   {
      RvR_rw_write_u8(c->dst,c->flag);
      for(int i = 0;i<8;i++)
         if(!(c->flag&(1<<i)))
            RvR_rw_write_u8(c->dst,c->byte_buf[i]);

      c->byte = 0;
      c->flag = 0;
   }
}

void RvR_ppp_compress_flush(RvR_ppp_context *c, uint8_t pad)
{
   while(c->byte!=0)
      RvR_ppp_compress_push(c,pad);
}

#endif
#endif
