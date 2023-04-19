# RvR_ppp_ccontext

Context for ppp compression. Due to the size of the struct, you might consider allocating memory for it instead of putting it on the stack.

## Definition

```c
typedef struct
{
   uint32_t hash;
   uint8_t byte;
   uint8_t flag;

   RvR_rw *dst;
   uint8_t byte_buf[8];
   uint8_t guess_table[1 << 18];
}RvR_ppp_ccontext;
```

## Related functions

[RvR_ppp_dcontext](/rvr/rvr/ppp_dcontext)

[RvR_ppp_compress_init](/rvr/rvr/ppp_compress_init)

[RvR_ppp_compress_push](/rvr/rvr/ppp_compress_push)

[RvR_ppp_compress_flush](/rvr/rvr/ppp_compress_flush)

[RvR_rw](/rvr/rvr/rw)
