/*
RvnicRaven - data (de-)compression

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_COMPRESS_H_

#define _RVR_COMPRESS_H_

typedef struct
{
   uint32_t hash;
   uint8_t byte;
   uint8_t flag;

   RvR_rw *dst;
   uint8_t byte_buf[8];
   uint8_t guess_table[1 << 18];
}RvR_ppp_ccontext;

typedef struct
{
   uint32_t hash;
   uint8_t byte;
   uint8_t flag;

   RvR_rw *src;
   uint8_t guess_table[1 << 18];
}RvR_ppp_dcontext;

void RvR_crush_compress(RvR_rw *in, RvR_rw *out, unsigned level);

//Returns a TEMPORARY buffer tagged RVR_MALLOC_CACHE, copy the output to your own buffer
void *RvR_crush_decompress(RvR_rw *in, size_t *length);

void RvR_ppp_compress_init(RvR_ppp_ccontext *c, RvR_rw *dst);
void RvR_ppp_compress_push(RvR_ppp_ccontext *c, uint8_t byte);
void RvR_ppp_compress_flush(RvR_ppp_ccontext *c, uint8_t pad);

void RvR_ppp_decompress_init(RvR_ppp_dcontext *c, RvR_rw *src);
int16_t RvR_ppp_decompress_pop(RvR_ppp_dcontext *c);

#endif
