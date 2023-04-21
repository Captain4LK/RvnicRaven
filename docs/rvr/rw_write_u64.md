# RvR_rw_write_u64

Writes an uint64_t to the RvR_rw struct, adjusting for endiannes. Returns amount of bytes written.

## Definition

```c
int RvR_rw_write_u64(RvR_rw *rw, uint64_t val);
```

## Related

[RvR_rw_read_u64](/rvr/rvr/rw_read_u64)
