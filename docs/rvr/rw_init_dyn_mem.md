# RvR_rw_init_dyn_mem

Initializes the provided RvR_rw struct to a dynamic array. Base size is base_len, buffer will be expanded by at least min_grow when reaching the end of the buffer while writing.

## Definition

```c
void RvR_rw_init_dyn_mem(RvR_rw *rw, size_t base_len, size_t min_grow);
```

## Related

[RvR_rw_init_const_mem](/rvr/rvr/rw_init_const_mem)

[RvR_rw_init_mem](/rvr/rvr/rw_init_mem)
