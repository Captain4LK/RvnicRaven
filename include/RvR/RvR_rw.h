/*
RvnicRaven - file abstraction

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_RW_H_

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

typedef void (*RvR_rw_usr_init)  (RvR_rw *rw, void *data);
typedef void (*RvR_rw_usr_close) (RvR_rw *rw);
typedef void (*RvR_rw_usr_flush) (RvR_rw *rw);
typedef int (*RvR_rw_usr_seek)  (RvR_rw *rw, long offset, int origin);
typedef long (*RvR_rw_usr_tell)  (RvR_rw *rw);
typedef int (*RvR_rw_usr_eof)   (RvR_rw *rw);
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
      } mem;

      struct
      {
         void *mem;
         long size;
         long csize;
         long pos;
         long min_grow;
      } dmem;

      struct
      {
         const void *mem;
         long size;
         long pos;
      } cmem;

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
      } usr;
   } as;
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
