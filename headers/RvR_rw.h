#ifndef _RVR_RW_H_

/*
   RvnicRaven - data stream abstraction

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define RVR_RW_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

/*
   malloc(), realloc(), free() can be overwritten by 
   defining the following macros:

   RVR_RW_MALLOC
   RVR_RW_FREE
   RVR_RW_REALLOC
*/

#define _RVR_RW_H_

typedef enum
{
   RVR_RW_INVALID = 0,
   RVR_RW_STD_FILE = 1,
   RVR_RW_STD_FILE_PATH = 2,
   RVR_RW_MEM = 3,
   RVR_RW_DYN_MEM = 4,
   RVR_RW_CONST_MEM = 5,
   RVR_RW_USR = 6,
}RvR_rw_type;

typedef enum
{
   RVR_RW_LITTLE_ENDIAN = 0,
   RVR_RW_BIG_ENDIAN = 1,
}RvR_rw_endian;

typedef struct RvR_rw RvR_rw;

typedef void   (*RvR_rw_usr_init)  (RvR_rw *rw, void *data);
typedef void   (*RvR_rw_usr_close) (RvR_rw *rw);
typedef void   (*RvR_rw_usr_flush) (RvR_rw *rw);
typedef int    (*RvR_rw_usr_seek)  (RvR_rw *rw, long offset, int origin);
typedef long   (*RvR_rw_usr_tell)  (RvR_rw *rw);
typedef int    (*RvR_rw_usr_eof)   (RvR_rw *rw);
typedef size_t (*RvR_rw_usr_read)  (RvR_rw *rw, void *buffer, size_t size, size_t count);
typedef size_t (*RvR_rw_usr_write) (RvR_rw *rw, const void *buffer, size_t size, size_t count);

struct RvR_rw
{
   RvR_rw_type type;
   RvR_rw_endian endian;

   union
   {
      FILE *fp;
      struct
      {
         void *mem;
         long size;
         long csize;
         long pos;
      }mem;
      struct
      {
         void *mem;
         long size;
         long csize;
         long pos;
         long min_grow;
      }dmem;
      struct
      {
         const void *mem;
         long size;
         long pos;
      }cmem;
      struct
      {
         RvR_rw_usr_close close;
         RvR_rw_usr_flush flush;
         RvR_rw_usr_seek seek;
         RvR_rw_usr_tell tell;
         RvR_rw_usr_eof eof;
         RvR_rw_usr_read read;
         RvR_rw_usr_write write;

         void *data;
      }usr;
   }as;
};

void RvR_rw_init_file(RvR_rw *rw, FILE *f);
void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode);
void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len, size_t clen);
void RvR_rw_init_dyn_mem(RvR_rw *rw, size_t base_len, size_t min_grow);
void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len);
void RvR_rw_init_usr(RvR_rw *rw, RvR_rw_usr_init init, void *data);

int    RvR_rw_valid(const RvR_rw *rw);
void   RvR_rw_endian_set(RvR_rw *rw, RvR_rw_endian endian);
void   RvR_rw_close(RvR_rw *rw);
void   RvR_rw_flush(RvR_rw *rw);
int    RvR_rw_seek(RvR_rw *rw, long offset, int origin);
long   RvR_rw_tell(RvR_rw *rw);
int    RvR_rw_eof(RvR_rw *rw);
size_t RvR_rw_read(RvR_rw *rw, void *buffer, size_t size, size_t count);
size_t RvR_rw_write(RvR_rw *rw, const void *buffer, size_t size, size_t count);
int    RvR_rw_printf(RvR_rw *rw, const char *format, ...);

int RvR_rw_write_u8(RvR_rw *rw, uint8_t val);
int RvR_rw_write_u16(RvR_rw *rw, uint16_t val);
int RvR_rw_write_u32(RvR_rw *rw, uint32_t val);
int RvR_rw_write_u64(RvR_rw *rw, uint64_t val);

uint8_t  RvR_rw_read_u8(RvR_rw *rw);
uint16_t RvR_rw_read_u16(RvR_rw *rw);
uint32_t RvR_rw_read_u32(RvR_rw *rw);
uint64_t RvR_rw_read_u64(RvR_rw *rw);

#endif

#ifdef RVR_RW_IMPLEMENTATION
#ifndef RVR_RW_IMPLEMENTATION_ONCE
#define RVR_RW_IMPLEMENTATION_ONCE

#ifndef RVR_RW_MALLOC
#define RVR_RW_MALLOC malloc
#endif

#ifndef RVR_RW_FREE
#define RVR_RW_FREE free
#endif

#ifndef RVR_RW_REALLOC
#define RVR_RW_REALLOC realloc
#endif

#define RVR_RW_MAX(a,b) ((a)>(b)?(a):(b))

void RvR_rw_init_file(RvR_rw *rw, FILE *f)
{
   if(rw==NULL||f==NULL)
      return;

   rw->type = RVR_RW_STD_FILE;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   rw->as.fp = f;
}

void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode)
{
   if(rw==NULL||path==NULL||mode==NULL)
      return;

   rw->type = RVR_RW_STD_FILE_PATH;
   rw->endian = RVR_RW_LITTLE_ENDIAN;
   rw->as.fp = fopen(path,mode);
   if(rw->as.fp==NULL)
      rw->type = RVR_RW_INVALID;
}

void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len, size_t clen)
{
   if(rw==NULL||mem==NULL)
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
   rw->as.dmem.mem = RVR_RW_MALLOC(base_len);
   rw->as.dmem.size = base_len;
   rw->as.dmem.csize = 0;
   rw->as.dmem.pos = 0;
   rw->as.dmem.min_grow = min_grow;
}

void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len)
{
   if(rw==NULL||mem==NULL)
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
   init(rw,data);
}

int RvR_rw_valid(const RvR_rw *rw)
{
   if(rw==NULL)
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
   if(rw->type==RVR_RW_DYN_MEM)
      RVR_RW_FREE(rw->as.dmem.mem);
   else if(rw->type==RVR_RW_USR)
      rw->as.usr.flush(rw);
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
      return fseek(rw->as.fp,offset,origin);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.mem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.mem.pos+=offset;
      else if(origin==SEEK_END)
         rw->as.mem.pos = rw->as.mem.csize+offset;

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
         rw->as.dmem.pos+=offset;
      else if(origin==SEEK_END)
         rw->as.dmem.pos = rw->as.dmem.csize+offset;

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
         rw->as.cmem.pos+=offset;
      else if(origin==SEEK_END)
         rw->as.cmem.pos = rw->as.cmem.size+offset;

      if(rw->as.cmem.pos<0)
      {
         rw->as.cmem.pos = 0;
         return 1;
      }

      return 0;
   }
   else if(rw->type==RVR_RW_USR)
   {
      return rw->as.usr.seek(rw,offset,origin);
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
      return fread(buffer,size,count,rw->as.fp);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      uint8_t *buff_out = buffer;
      uint8_t *buff_in = rw->as.mem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.mem.pos+(long)size>rw->as.mem.csize)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->as.mem.pos,size);
         rw->as.mem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      uint8_t *buff_out = buffer;
      uint8_t *buff_in = rw->as.dmem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.dmem.pos+(long)size>rw->as.dmem.csize)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->as.dmem.pos,size);
         rw->as.dmem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      uint8_t *buff_out = buffer;
      const uint8_t *buff_in = rw->as.cmem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.cmem.pos+(long)size>rw->as.cmem.size)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->as.cmem.pos,size);
         rw->as.cmem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_USR)
   {
      return rw->as.usr.read(rw,buffer,size,count);
   }
   

   return 0;
}

size_t RvR_rw_write(RvR_rw *rw, const void *buffer, size_t size, size_t count)
{
   if(rw==NULL||buffer==NULL||!RvR_rw_valid(rw))
      return 0;

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fwrite(buffer,size,count,rw->as.fp);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      uint8_t *buff_out = rw->as.mem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.mem.pos+size>rw->as.mem.size)
            return 1;

         rw->as.mem.csize = RVR_RW_MAX(rw->as.mem.csize,rw->as.mem.pos);
         memcpy(buff_out+rw->as.mem.pos,buff_in+(i*size),size);
         rw->as.mem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      uint8_t *buff_out = rw->as.dmem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.dmem.pos+size>rw->as.dmem.size)
         {
            rw->as.dmem.size+=RVR_RW_MAX(rw->as.dmem.min_grow,rw->as.dmem.pos+(long)size-rw->as.dmem.size);
            rw->as.dmem.mem = RVR_RW_REALLOC(rw->as.dmem.mem,rw->as.dmem.size);
         }

         rw->as.dmem.csize = RVR_RW_MAX(rw->as.dmem.csize,rw->as.dmem.pos);
         memcpy(buff_out+rw->as.dmem.pos,buff_in+(i*size),size);
         rw->as.dmem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      return 0;
   }
   else if(rw->type==RVR_RW_USR)
   {
      return rw->as.usr.write(rw,buffer,size,count);
   }

   return 0;
}

int RvR_rw_write_u8(RvR_rw *rw, uint8_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   return RvR_rw_write(rw,&val,1,1);
}

int RvR_rw_printf(RvR_rw *rw, const char *format, ...)
{
   if(rw==NULL||!RvR_rw_valid(rw)||format==NULL)
      return -1;

   //Limit of 1024 characters
   char tmp[1024];

   va_list args;
   va_start(args,format);
   int ret = vsnprintf(tmp,1024,format,args);
   va_end(args);

   RvR_rw_write(rw,tmp,strlen(tmp),1);

   return ret;
}

int RvR_rw_write_u16(RvR_rw *rw, uint16_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   int res = 0;
   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
   {
      res+=RvR_rw_write_u8(rw,(val)&255);
      res+=RvR_rw_write_u8(rw,(val>>8)&255);
   }
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
   {
      res+=RvR_rw_write_u8(rw,(val>>8)&255);
      res+=RvR_rw_write_u8(rw,(val)&255);
   }

   return res;
}

int RvR_rw_write_u32(RvR_rw *rw, uint32_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   int res = 0;
   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
   {
      res+=RvR_rw_write_u8(rw,(val)&255);
      res+=RvR_rw_write_u8(rw,(val>>8)&255);
      res+=RvR_rw_write_u8(rw,(val>>16)&255);
      res+=RvR_rw_write_u8(rw,(val>>24)&255);
   }
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
   {
      res+=RvR_rw_write_u8(rw,(val>>24)&255);
      res+=RvR_rw_write_u8(rw,(val>>16)&255);
      res+=RvR_rw_write_u8(rw,(val>>8)&255);
      res+=RvR_rw_write_u8(rw,(val)&255);
   }

   return res;
}

int RvR_rw_write_u64(RvR_rw *rw, uint64_t val)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   int res = 0;
   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
   {
      res+=RvR_rw_write_u8(rw,(val)&255);
      res+=RvR_rw_write_u8(rw,(val>>8)&255);
      res+=RvR_rw_write_u8(rw,(val>>16)&255);
      res+=RvR_rw_write_u8(rw,(val>>24)&255);
      res+=RvR_rw_write_u8(rw,(val>>32)&255);
      res+=RvR_rw_write_u8(rw,(val>>40)&255);
      res+=RvR_rw_write_u8(rw,(val>>48)&255);
      res+=RvR_rw_write_u8(rw,(val>>56)&255);
   }
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
   {
      res+=RvR_rw_write_u8(rw,(val>>56)&255);
      res+=RvR_rw_write_u8(rw,(val>>48)&255);
      res+=RvR_rw_write_u8(rw,(val>>40)&255);
      res+=RvR_rw_write_u8(rw,(val>>32)&255);
      res+=RvR_rw_write_u8(rw,(val>>24)&255);
      res+=RvR_rw_write_u8(rw,(val>>16)&255);
      res+=RvR_rw_write_u8(rw,(val>>8)&255);
      res+=RvR_rw_write_u8(rw,(val)&255);
   }

   return res;
}

uint8_t RvR_rw_read_u8(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   uint8_t b0 = 0;
   RvR_rw_read(rw,&b0,1,1);

   return b0;
}

uint16_t RvR_rw_read_u16(RvR_rw *rw)
{
   if(rw==NULL||!RvR_rw_valid(rw))
      return 0;

   uint16_t b0 = RvR_rw_read_u8(rw);
   uint16_t b1 = RvR_rw_read_u8(rw);

   if(rw->endian==RVR_RW_LITTLE_ENDIAN)
      return (b1<<8)|(b0);
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
      return (b0<<8)|(b1);

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
      return (b3<<24)|(b2<<16)|(b1<<8)|(b0);
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
      return (b0<<24)|(b1<<16)|(b2<<8)|(b3);

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
      return (b7<<56)|(b6<<48)|(b5<<40)|(b4<<32)|(b3<<24)|(b2<<16)|(b1<<8)|(b0);
   else if(rw->endian==RVR_RW_BIG_ENDIAN)
      return (b0<<56)|(b1<<48)|(b2<<40)|(b3<<32)|(b4<<24)|(b5<<16)|(b6<<8)|(b7);

   return 0;
}

#undef RVR_RW_MAX

#endif
#endif
