# RvR_rw

Struct for RvR_rw context

## Definition

```c
typedef struct
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
}RvR_rw;

```

## Related

[RvR_rw_type](/rvr/rvr/rw_type)

[RvR_rw](/rvr/rvr/rw)
