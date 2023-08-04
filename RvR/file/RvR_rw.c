/*
RvnicRaven - file abstraction

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
//-------------------------------------

//Internal includes
#include "RvR/RvR_log.h"
#include "RvR/RvR_malloc.h"
#include "RvR/RvR_rw.h"
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

void RvR_rw_init_file(RvR_rw *rw, FILE *f)
{
   if(rw==NULL)
      return;

   rw->type = RVR_RW_INVALID;
   if(f==NULL)
      return;

   rw->type = RVR_RW_STD_FILE;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   rw->as.fp = f;
}

void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode)
{
   if(rw==NULL)
      return;

   rw->type = RVR_RW_INVALID;
   if(path==NULL||mode==NULL)
      return;

   rw->type = RVR_RW_STD_FILE_PATH;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   rw->as.fp = fopen(path, mode);
   if(rw->as.fp==NULL)
      rw->type = RVR_RW_INVALID;
}

void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len, size_t clen)
{
   if(rw==NULL)
      return;

   rw->type = RVR_RW_INVALID;
   if(mem==NULL)
      return;

   rw->type = RVR_RW_MEM;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   rw->as.mem.mem = mem;
   rw->as.mem.size = len;
   rw->as.mem.pos = 0;
   rw->as.mem.csize = clen;
}

void RvR_rw_init_dyn_mem(RvR_rw *rw, size_t base_len, size_t min_grow)
{
   if(rw==NULL)
      return;

   rw->type = RVR_RW_DYN_MEM;
   rw->as.dmem.mem = RvR_malloc(base_len, "RvR_rw dynamic memory rw");
   rw->as.dmem.size = base_len;
   rw->as.dmem.csize = 0;
   rw->as.dmem.pos = 0;
   rw->as.dmem.min_grow = min_grow;
}

void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len)
{
   if(rw==NULL)
      return;

   rw->type = RVR_RW_INVALID;
   if(mem==NULL)
      return;

   rw->type = RVR_RW_CONST_MEM;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   rw->as.cmem.mem = mem;
   rw->as.cmem.size = len;
   rw->as.cmem.pos = 0;
}

void RvR_rw_init_usr(RvR_rw *rw, RvR_rw_usr_init init, void *data)
{
   rw->type = RVR_RW_USR;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   init(rw, data);
}

int RvR_rw_valid(const RvR_rw *rw)
{
   if(rw==NULL)
      return 0;

   //Invalid file pointer
   if((rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)&&rw->as.fp==0)
      return 0;

   return rw->type!=RVR_RW_INVALID;
}

void RvR_rw_endian_set(RvR_rw *rw, RvR_rw_endian endian)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return;
   rw->endian = endian;
}

void RvR_rw_close(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return;

   if(rw->type==RVR_RW_STD_FILE_PATH)
      fclose(rw->as.fp);
   else if(rw->type==RVR_RW_DYN_MEM)
      RvR_free(rw->as.dmem.mem);
   else if(rw->type==RVR_RW_USR)
      rw->as.usr.flush(rw);

   rw->type = RVR_RW_INVALID;
}

void RvR_rw_flush(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      fflush(rw->as.fp);
   else if(rw->type==RVR_RW_USR)
      rw->as.usr.flush(rw);
}

int RvR_rw_seek(RvR_rw *rw, long offset, int origin)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 1;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fseek(rw->as.fp, offset, origin);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.mem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.mem.pos += offset;
      else if(origin==SEEK_END)
         rw->as.mem.pos = rw->as.mem.csize + offset;

      if(rw->as.mem.pos<0)
      {
         rw->as.mem.pos = 0;
         return 1;
      }

      return 0;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.dmem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.dmem.pos += offset;
      else if(origin==SEEK_END)
         rw->as.dmem.pos = rw->as.dmem.csize + offset;

      if(rw->as.dmem.pos<0)
      {
         rw->as.dmem.pos = 0;
         return 1;
      }

      return 0;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.cmem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.cmem.pos += offset;
      else if(origin==SEEK_END)
         rw->as.cmem.pos = rw->as.cmem.size + offset;

      if(rw->as.cmem.pos<0)
      {
         rw->as.cmem.pos = 0;
         return 1;
      }

      return 0;
   }
   else if(rw->type==RVR_RW_USR)
   {
      return rw->as.usr.seek(rw, offset, origin);
   }

   return 1;
}

long RvR_rw_tell(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return EOF;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      return ftell(rw->as.fp);
   else if(rw->type==RVR_RW_MEM)
      return rw->as.mem.pos;
   else if(rw->type==RVR_RW_DYN_MEM)
      return rw->as.dmem.pos;
   else if(rw->type==RVR_RW_CONST_MEM)
      return rw->as.cmem.pos;
   else if(rw->type==RVR_RW_USR)
      return rw->as.usr.tell(rw);

   return EOF;
}

int RvR_rw_eof(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 1;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      return feof(rw->as.fp);
   else if(rw->type==RVR_RW_MEM)
      return rw->as.mem.pos>=rw->as.mem.csize;
   else if(rw->type==RVR_RW_DYN_MEM)
      return rw->as.dmem.pos>=rw->as.dmem.csize;
   else if(rw->type==RVR_RW_CONST_MEM)
      return rw->as.cmem.pos>=rw->as.cmem.size;
   else if(rw->type==RVR_RW_USR)
      return rw->as.usr.eof(rw);

   return 1;
}

size_t RvR_rw_read(RvR_rw *rw, void *buffer, size_t size, size_t count)
{
   if(rw==NULL||buffer==NULL||!RvR_rw_valid(rw))
      return 0;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fread(buffer, size, count, rw->as.fp);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      uint8_t *buff_out = buffer;
      uint8_t *buff_in = rw->as.mem.mem;

      for(size_t i = 0; i<count; i++)
      {
         if(rw->as.mem.pos + (long)size>rw->as.mem.csize)
            return i;

         memcpy(buff_out + (i * size), buff_in + rw->as.mem.pos, size);
         rw->as.mem.pos += size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      uint8_t *buff_out = buffer;
      uint8_t *buff_in = rw->as.dmem.mem;

      for(size_t i = 0; i<count; i++)
      {
         if(rw->as.dmem.pos + (long)size>rw->as.dmem.csize)
            return i;

         memcpy(buff_out + (i * size), buff_in + rw->as.dmem.pos, size);
         rw->as.dmem.pos += size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      uint8_t *buff_out = buffer;
      const uint8_t *buff_in = rw->as.cmem.mem;

      for(size_t i = 0; i<count; i++)
      {
         if(rw->as.cmem.pos + (long)size>rw->as.cmem.size)
            return i;

         memcpy(buff_out + (i * size), buff_in + rw->as.cmem.pos, size);
         rw->as.cmem.pos += size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_USR)
   {
      return rw->as.usr.read(rw, buffer, size, count);
   }


   return 0;
}

size_t RvR_rw_write(RvR_rw *rw, const void *buffer, size_t size, size_t count)
{
   if(rw==NULL||buffer==NULL||!RvR_rw_valid(rw))
      return 0;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fwrite(buffer, size, count, rw->as.fp);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      uint8_t *buff_out = rw->as.mem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0; i<count; i++)
      {
         if(rw->as.mem.pos + (long)size>rw->as.mem.size)
            return 1;

         rw->as.mem.csize = RvR_max(rw->as.mem.csize, rw->as.mem.pos);
         memcpy(buff_out + rw->as.mem.pos, buff_in + (i * size), size);
         rw->as.mem.pos += size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      uint8_t *buff_out = rw->as.dmem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0; i<count; i++)
      {
         if(rw->as.dmem.pos + (long)size>rw->as.dmem.size)
         {
            rw->as.dmem.size += RvR_max(rw->as.dmem.min_grow, rw->as.dmem.pos + (long)size - rw->as.dmem.size);
            rw->as.dmem.mem = RvR_realloc(rw->as.dmem.mem, rw->as.dmem.size, "RvR_rw dynamic memory rw grow");
         }

         rw->as.dmem.csize = RvR_max(rw->as.dmem.csize, rw->as.dmem.pos);
         memcpy(buff_out + rw->as.dmem.pos, buff_in + (i * size), size);
         rw->as.dmem.pos += size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      return 0;
   }
   else if(rw->type==RVR_RW_USR)
   {
      return rw->as.usr.write(rw, buffer, size, count);
   }

   return 0;
}

int RvR_rw_printf(RvR_rw *rw, const char *format, ...)
{
   if(rw==NULL||!RvR_rw_valid(rw)||format==NULL)
      return -1;

   //Limit of 1024 characters
   char tmp[1024];

   va_list args;
   va_start(args, format);
   int ret = vsnprintf(tmp, 1024, format, args);
   va_end(args);

   RvR_rw_write(rw, tmp, strlen(tmp), 1);

   return ret;
}

size_t RvR_rw_write_u8(RvR_rw *rw, uint8_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   return RvR_rw_write(rw, &val, 1, 1);
}

size_t RvR_rw_write_u16(RvR_rw *rw, uint16_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   size_t res = 0;
   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
   {
      res += RvR_rw_write_u8(rw, (uint8_t)(val & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 8) & 255));
   }
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
   {
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 8) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)(val & 255));
   }

   return res;
}

size_t RvR_rw_write_u32(RvR_rw *rw, uint32_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   size_t res = 0;
   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
   {
      res += RvR_rw_write_u8(rw, (uint8_t)((val) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 8) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 16) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 24) & 255));
   }
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
   {
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 24) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 16) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 8) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val) & 255));
   }

   return res;
}

size_t RvR_rw_write_u64(RvR_rw *rw, uint64_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   size_t res = 0;
   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
   {
      res += RvR_rw_write_u8(rw, (uint8_t)((val) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 8) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 16) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 24) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 32) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 40) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 48) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 56) & 255));
   }
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
   {
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 56) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 48) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 40) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 32) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 24) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 16) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val >> 8) & 255));
      res += RvR_rw_write_u8(rw, (uint8_t)((val) & 255));
   }

   return res;
}

uint8_t RvR_rw_read_u8(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   uint8_t b0 = 0;
   RvR_rw_read(rw, &b0, 1, 1);

   return b0;
}

uint16_t RvR_rw_read_u16(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   uint16_t b0 = RvR_rw_read_u8(rw);
   uint16_t b1 = RvR_rw_read_u8(rw);

   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
      return (b1 << 8) | (b0);
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
      return (b0 << 8) | (b1);

   return 0;
}

uint32_t RvR_rw_read_u32(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   uint32_t b0 = RvR_rw_read_u8(rw);
   uint32_t b1 = RvR_rw_read_u8(rw);
   uint32_t b2 = RvR_rw_read_u8(rw);
   uint32_t b3 = RvR_rw_read_u8(rw);

   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
      return (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0);
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
      return (b0 << 24) | (b1 << 16) | (b2 << 8) | (b3);

   return 0;
}

uint64_t RvR_rw_read_u64(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   uint64_t b0 = RvR_rw_read_u8(rw);
   uint64_t b1 = RvR_rw_read_u8(rw);
   uint64_t b2 = RvR_rw_read_u8(rw);
   uint64_t b3 = RvR_rw_read_u8(rw);
   uint64_t b4 = RvR_rw_read_u8(rw);
   uint64_t b5 = RvR_rw_read_u8(rw);
   uint64_t b6 = RvR_rw_read_u8(rw);
   uint64_t b7 = RvR_rw_read_u8(rw);

   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
      return (b7 << 56) | (b6 << 48) | (b5 << 40) | (b4 << 32) | (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0);
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
      return (b0 << 56) | (b1 << 48) | (b2 << 40) | (b3 << 32) | (b4 << 24) | (b5 << 16) | (b6 << 8) | (b7);

   return 0;
}
//-------------------------------------
