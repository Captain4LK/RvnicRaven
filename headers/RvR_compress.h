#ifndef _RVR_COMPRESS_H_

/*
   RvnicRaven - data (de-)compression

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define RVR_RW_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_COMPRESS_H_

void  RvR_compress(RvR_rw *in, RvR_rw *out, unsigned level);
void *RvR_decompress(RvR_rw *in, int32_t *length);

#endif

#ifdef RVR_COMPRESS_IMPLEMENTATION
#ifndef RVR_COMPRESS_IMPLEMENTATION_ONCE
#define RVR_COMPRESS_IMPLEMENTATION_ONCE

#ifndef RvR_log
#define RvR_log(...) while(0)
#endif

#ifndef RvR_error_fail
#define RvR_error_fail(w,...) do { goto RvR_err; } while(0)
#endif

#ifndef RvR_error_check
#define RvR_error_check(X,w,...) do { if(!(X)) RvR_error_fail(w,__VA_ARGS__); } while(0)
#endif

#ifndef RvR_log_line
#define RvR_log_line(w,...) while(0)
#endif

#define RvR_comp_min(a,b) ((a)<(b)?(a):(b))
#define RvR_comp_max(a,b) ((a)>(b)?(a):(b))

enum
{
   RVR_COMP_W_BITS = 21, //Window size [17,23]
   RVR_COMP_W_SIZE = 1<<RVR_COMP_W_BITS,
   RVR_COMP_W_MASK = RVR_COMP_W_SIZE-1,
   RVR_COMP_SLOT_BITS = 4,
   RVR_COMP_NUM_SLOTS = 1<<RVR_COMP_SLOT_BITS,

   RVR_COMP_A_BITS = 2, //1 xx
   RVR_COMP_B_BITS = 2, //01 xx
   RVR_COMP_C_BITS = 2, //001 xx
   RVR_COMP_D_BITS = 3, //0001 xxx
   RVR_COMP_E_BITS = 5, //00001 xxxxx
   RVR_COMP_F_BITS = 9, //00000 xxxxxxxxx
   RVR_COMP_A = 1<<RVR_COMP_A_BITS,
   RVR_COMP_B = (1<<RVR_COMP_B_BITS)+RVR_COMP_A,
   RVR_COMP_C = (1<<RVR_COMP_C_BITS)+RVR_COMP_B,
   RVR_COMP_D = (1<<RVR_COMP_D_BITS)+RVR_COMP_C,
   RVR_COMP_E = (1<<RVR_COMP_E_BITS)+RVR_COMP_D,
   RVR_COMP_F = (1<<RVR_COMP_F_BITS)+RVR_COMP_E,
   RVR_COMP_MIN_MATCH = 3,
   RVR_COMP_MAX_MATCH = (RVR_COMP_F-1)+RVR_COMP_MIN_MATCH,

   RVR_COMP_TOO_FAR = 1<<16,

   RVR_COMP_HASH1_LEN = RVR_COMP_MIN_MATCH,
   RVR_COMP_HASH2_LEN = RVR_COMP_MIN_MATCH+1,
   RVR_COMP_HASH1_BITS = 21,
   RVR_COMP_HASH2_BITS = 24,
   RVR_COMP_HASH1_SIZE = 1<<RVR_COMP_HASH1_BITS,
   RVR_COMP_HASH2_SIZE = 1<<RVR_COMP_HASH2_BITS,
   RVR_COMP_HASH1_MASK = RVR_COMP_HASH1_SIZE-1,
   RVR_COMP_HASH2_MASK = RVR_COMP_HASH2_SIZE-1,
   RVR_COMP_HASH1_SHIFT = (RVR_COMP_HASH1_BITS+RVR_COMP_HASH1_LEN-1)/RVR_COMP_HASH1_LEN,
   RVR_COMP_HASH2_SHIFT = (RVR_COMP_HASH2_BITS+RVR_COMP_HASH2_LEN-1)/RVR_COMP_HASH2_LEN,
};

typedef struct
{
   RvR_rw *inbuf;
   RvR_rw *outbuf;
   unsigned bit_buf;
   unsigned bit_count;
}rvr_comp_bits;

static void     rvr_comp_crush_rvr_compress(const uint8_t *buf, size_t size, RvR_rw *outbuf, size_t level);
static void     rvr_comp_crush_dervr_compress(RvR_rw *inbuf, uint8_t *outbuf, uint32_t outlen);
static int      rvr_comp_update_hash1(int h, int c);
static int      rvr_comp_update_hash2(int h, int c);
static int      rvr_comp_get_penalty(int a, int b);
static void     rvr_comp_bits_init(rvr_comp_bits *b, RvR_rw *inbuf, RvR_rw *outbuf);
static void     rvr_comp_bits_put(rvr_comp_bits *b, unsigned n, unsigned x);
static void     rvr_comp_bits_flush(rvr_comp_bits *b);
static unsigned rvr_comp_bits_get(rvr_comp_bits *b, unsigned n);

void RvR_compress(RvR_rw *in, RvR_rw *out, unsigned level)
{
   uint8_t *buffer_in = NULL;
   int32_t size = 0;

   RvR_rw_endian_set(in,RVR_RW_LITTLE_ENDIAN);
   RvR_rw_seek(in,0,SEEK_END);
   size = RvR_rw_tell(in);
   RvR_rw_seek(in,0,SEEK_SET);

   buffer_in = RvR_malloc(size+1);
   RvR_rw_read(in,buffer_in,size,1);
   buffer_in[size] = 0;

   RvR_rw_seek(out,0,SEEK_END);
   RvR_rw_write_u32(out,size);
   rvr_comp_crush_rvr_compress(buffer_in,size,out,level);

   RvR_free(buffer_in);
}

void *RvR_decompress(RvR_rw *in, int32_t *length)
{
   RvR_rw_seek(in,0,SEEK_SET);
   RvR_rw_endian_set(in,RVR_RW_LITTLE_ENDIAN);
   *length = RvR_rw_read_u32(in);

   uint8_t *buffer_out = RvR_malloc((*length)+1);
   rvr_comp_crush_dervr_compress(in,buffer_out,*length);
   buffer_out[*length] = 0;

   return buffer_out;
}

//rvr_comp_crush_rvr_compress, rvr_comp_crush_dervr_compress, rvr_comp_update_hash1, rvr_comp_update_hash2, rvr_comp_bits_init, rvr_comp_get_penalty, rvr_comp_bits_put, rvr_comp_bits_get, rvr_comp_bits_flush
//by r-lyeh(https://github.com/r-lyeh), stdpack.c (https://github.com/r-lyeh/stdpack.c/blob/master/src/crush.c)
//Original license info:
// crush.cpp
// Written and placed in the public domain by Ilya Muravyov
// Additional code by @r-lyeh (public domain). @todo: honor unused args inlen/outlen

static void rvr_comp_crush_rvr_compress(const uint8_t *buf, size_t size, RvR_rw *outbuf, size_t level)
{
   static int head[RVR_COMP_HASH1_SIZE+RVR_COMP_HASH2_SIZE];
   static int prev[RVR_COMP_W_SIZE];

   const int max_chain[11] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 1<<12}; //[0fastest..10uber]
   level = RvR_comp_min(level,11);

   rvr_comp_bits bits;

   for(int i = 0;i<RVR_COMP_HASH1_SIZE+RVR_COMP_HASH2_SIZE;i++)
      head[i]=-1;

   int h1 = 0;
   int h2 = 0;
   for(int i = 0;i<RVR_COMP_HASH1_LEN;i++)
      h1 = rvr_comp_update_hash1(h1, buf[i]);
   for(int i = 0;i<RVR_COMP_HASH2_LEN;i++)
      h2 = rvr_comp_update_hash2(h2,buf[i]);

   rvr_comp_bits_init(&bits, NULL, outbuf);

   size_t p = 0;
   while (p<size)
   {
      int len = RVR_COMP_MIN_MATCH-1;
      int offset = RVR_COMP_W_SIZE;

      const int max_match = RvR_comp_min((int)RVR_COMP_MAX_MATCH,(int)size-p);
      const int limit = RvR_comp_max((int)p-RVR_COMP_W_SIZE,0);

      if(head[h1]>=limit)
      {
         int s = head[h1];
         if(buf[s]==buf[p])
         {
            int l = 0;
            while(++l<max_match)
               if(buf[s+l]!=buf[p+l])
                  break;
            if(l>len)
            {
               len = l;
               offset = p-s;
            }
         }
      }

      if(len<RVR_COMP_MAX_MATCH)
      {
         int chain_len = max_chain[level];
         int s = head[h2+RVR_COMP_HASH1_SIZE];

         while((chain_len--!=0)&&(s>=limit))
         {
            if((buf[s+len]==buf[p+len])&&(buf[s]==buf[p]))
            {
               int l = 0;
               while(++l<max_match)
                  if(buf[s+l]!=buf[p+l])
                     break;
               if(l>len+rvr_comp_get_penalty((p-s)>>4,offset))
               {
                  len = l;
                  offset=p-s;
               }
               if(l==max_match)
                  break;
            }
            s = prev[s&RVR_COMP_W_MASK];
         }
      }

      if((len==RVR_COMP_MIN_MATCH)&&(offset>RVR_COMP_TOO_FAR))
         len = 0;

      if((level>=2)&&(len>=RVR_COMP_MIN_MATCH)&&(len<max_match))
      {
         const int next_p = p+1;
         const int max_lazy = RvR_comp_min((int)len+4,(int)max_match);

         int chain_len = max_chain[level];
         int s = head[rvr_comp_update_hash2(h2, buf[next_p+(RVR_COMP_HASH2_LEN-1)])+RVR_COMP_HASH1_SIZE];

         while((chain_len--!=0)&&(s>=limit))
         {
            if((buf[s+len]==buf[next_p+len])&&(buf[s]==buf[next_p]))
            {
               int l = 0;
               while(++l<max_lazy)
                  if(buf[s+l]!=buf[next_p+l])
                     break;
               if(l>len+rvr_comp_get_penalty(next_p-s, offset))
               {
                  len=0;
                  break;
               }
               if(l==max_lazy)
                  break;
            }
            s = prev[s&RVR_COMP_W_MASK];
         }
      }

      if(len>=RVR_COMP_MIN_MATCH) //Match
      {
         rvr_comp_bits_put(&bits,1,1);

         const int l = len-RVR_COMP_MIN_MATCH;
         if(l<RVR_COMP_A)
         {
            rvr_comp_bits_put(&bits,1,1); //1
            rvr_comp_bits_put(&bits,RVR_COMP_A_BITS,l);
         }
         else if(l<RVR_COMP_B)
         {
            rvr_comp_bits_put(&bits,2,1<<1); //01
            rvr_comp_bits_put(&bits,RVR_COMP_B_BITS,l-RVR_COMP_A);
         }
         else if(l<RVR_COMP_C)
         {
            rvr_comp_bits_put(&bits,3,1<<2); //001
            rvr_comp_bits_put(&bits,RVR_COMP_C_BITS,l-RVR_COMP_B);
         }
         else if(l<RVR_COMP_D)
         {
            rvr_comp_bits_put(&bits,4,1<<3); //0001
            rvr_comp_bits_put(&bits,RVR_COMP_D_BITS,l-RVR_COMP_C);
         }
         else if(l<RVR_COMP_E)
         {
            rvr_comp_bits_put(&bits,5,1<<4); //00001
            rvr_comp_bits_put(&bits,RVR_COMP_E_BITS,l-RVR_COMP_D);
         }
         else
         {
            rvr_comp_bits_put(&bits,5,0); //00000
            rvr_comp_bits_put(&bits,RVR_COMP_F_BITS,l-RVR_COMP_E);
         }

         offset--;
         int log = RVR_COMP_W_BITS-RVR_COMP_NUM_SLOTS;
         while(offset>=(2<<log))
            log++;
         rvr_comp_bits_put(&bits,RVR_COMP_SLOT_BITS,log-(RVR_COMP_W_BITS-RVR_COMP_NUM_SLOTS));
         if(log>(RVR_COMP_W_BITS-RVR_COMP_NUM_SLOTS))
            rvr_comp_bits_put(&bits,log,offset-(1<<log));
         else
            rvr_comp_bits_put(&bits,RVR_COMP_W_BITS-(RVR_COMP_NUM_SLOTS-1),offset);
      }
      else //Literal
      {
         len = 1;
         rvr_comp_bits_put(&bits,9,buf[p]<<1); //0 xxxxxxxx
      }

      while(len--!=0) //Insert new strings
      {
         head[h1] = p;
         prev[p&RVR_COMP_W_MASK] = head[h2+RVR_COMP_HASH1_SIZE];
         head[h2+RVR_COMP_HASH1_SIZE] = p;
         p++;
         h1 = rvr_comp_update_hash1(h1,buf[p+(RVR_COMP_HASH1_LEN-1)]);
         h2 = rvr_comp_update_hash2(h2,buf[p+(RVR_COMP_HASH2_LEN-1)]);
      }
   }

   rvr_comp_bits_flush(&bits);
}

static void rvr_comp_crush_dervr_compress(RvR_rw *inbuf, uint8_t *outbuf, uint32_t outlen)
{
   unsigned p = 0;
   int s = 0;
   rvr_comp_bits bits;
   rvr_comp_bits_init(&bits,inbuf,NULL);

   while(p<outlen)
   {
      if(rvr_comp_bits_get(&bits,1))
      {
         unsigned len;
         if(rvr_comp_bits_get(&bits,1))      len = rvr_comp_bits_get(&bits,RVR_COMP_A_BITS);
         else if(rvr_comp_bits_get(&bits,1)) len = rvr_comp_bits_get(&bits,RVR_COMP_B_BITS)+RVR_COMP_A;
         else if(rvr_comp_bits_get(&bits,1)) len = rvr_comp_bits_get(&bits,RVR_COMP_C_BITS)+RVR_COMP_B;
         else if(rvr_comp_bits_get(&bits,1)) len = rvr_comp_bits_get(&bits,RVR_COMP_D_BITS)+RVR_COMP_C;
         else if(rvr_comp_bits_get(&bits,1)) len = rvr_comp_bits_get(&bits,RVR_COMP_E_BITS)+RVR_COMP_D;
         else                            len = rvr_comp_bits_get(&bits,RVR_COMP_F_BITS)+RVR_COMP_E;

         unsigned log = rvr_comp_bits_get(&bits,RVR_COMP_SLOT_BITS)+(RVR_COMP_W_BITS-RVR_COMP_NUM_SLOTS);
         if(log>RVR_COMP_W_BITS-RVR_COMP_NUM_SLOTS)
            s = rvr_comp_bits_get(&bits,log)+(1<<log);
         else
            s = rvr_comp_bits_get(&bits,RVR_COMP_W_BITS-RVR_COMP_NUM_SLOTS+1);
         s = ~s+p;

         RvR_error_check(s>=0&&s+len+3<=outlen,"RvR_dervr_compress","corrupted stream (s=%d p=%d): s out of bounds\n",s,p);
         RvR_error_check(p+len+3<=outlen,"RvR_dervr_compress","corrupted stream (s=%d p=%d): longer than specified by length\n",s,p);

         outbuf[p++] = outbuf[s++];
         outbuf[p++] = outbuf[s++];
         outbuf[p++] = outbuf[s++];
         while(len--!=0)
            outbuf[p++] = outbuf[s++];
      }
      else
      {
         outbuf[p++] = rvr_comp_bits_get(&bits,8);
         RvR_error_check(p<=outlen,"RvR_dervr_compress","corrupted stream (s=%d p=%d): longer than specified by length\n",s,p);
      }
   }

RvR_err:
   return;
}

static int rvr_comp_update_hash1(int h, int c)
{
   return ((h<<RVR_COMP_HASH1_SHIFT)+c)&RVR_COMP_HASH1_MASK;
}

static int rvr_comp_update_hash2(int h, int c)
{
   return ((h<<RVR_COMP_HASH2_SHIFT)+c)&RVR_COMP_HASH2_MASK;
}

static int rvr_comp_get_penalty(int a, int b)
{
   int p = 0;
   while(a>b)
   {
      a>>=3;
      p++;
   }

   return p;
}

static void rvr_comp_bits_init(rvr_comp_bits *b, RvR_rw *inbuf, RvR_rw *outbuf)
{
   b->bit_count = b->bit_buf = 0;
   b->inbuf = inbuf;
   b->outbuf = outbuf;
}

//Write n bits of value x to stream
static void rvr_comp_bits_put(rvr_comp_bits *b, unsigned n, unsigned x)
{
   b->bit_buf|=x<<b->bit_count;
   b->bit_count+=n;

   //Write filled bytes to output stream
   while(b->bit_count>=8)
   {
      RvR_rw_write_u8(b->outbuf,b->bit_buf);
      b->bit_buf>>=8;
      b->bit_count-=8;
   }
}

//Forces bit buffer flush
static void rvr_comp_bits_flush(rvr_comp_bits *b)
{
   rvr_comp_bits_put(b,7,0);
   b->bit_count = b->bit_buf = 0;
}

//Read n bits from input stream
static unsigned rvr_comp_bits_get(rvr_comp_bits *b, unsigned n)
{
   //Fill bit buffer from input stream
   while(b->bit_count<n)
   {
      b->bit_buf|=RvR_rw_read_u8(b->inbuf)<<b->bit_count;
      b->bit_count+=8;
   }

   unsigned x = b->bit_buf&((1<<n)-1);
   b->bit_buf>>=n;
   b->bit_count-=n;

   return x;
}

#undef RvR_comp_min
#undef RvR_comp_max

#endif
#endif
