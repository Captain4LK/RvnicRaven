# RvR_ppp_compress_push

Push a byte to the compressed stream. Every 8 pushes 1 to 9 bytes will be written to the output stream.

## Definition

```c
void RvR_ppp_compress_push(RvR_ppp_ccontext *c, uint8_t byte);
```

## Related

[RvR_ppp_ccontext](/rvr/rvr/ppp_ccontext)

[RvR_ppp_compress_init](/rvr/rvr/ppp_compress_init)

[RvR_ppp_compress_flush](/rvr/rvr/ppp_compress_flush)

[RvR_rw](/rvr/rvr/rw)
