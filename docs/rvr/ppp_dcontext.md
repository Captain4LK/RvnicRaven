# RvR_ppp_dcontext

Context for ppp decompression. Due to the size of the struct, you might consider allocating memory for it instead of putting it on the stack.

## Definition

```c
typedef struct
{
   uint32_t hash;
   uint8_t byte;
   uint8_t flag;

   RvR_rw *src;
   uint8_t guess_table[1 << 18];
}RvR_ppp_dcontext;
```

## Related functions

[RvR_ppp_ccontext](/rvr/rvr/ppp_dcontext)

[RvR_ppp_decompress_init](/rvr/rvr/ppp_decompress_init)

[RvR_ppp_decompress_pop](/rvr/rvr/ppp_decompress_pop)

[RvR_rw](/rvr/rvr/rw)
