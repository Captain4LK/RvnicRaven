# RvR_rw_init_mem

Initializes the provided RvR_rw struct using the provided memory buffer. len is the size of the buffer, clen how much the buffer is already filled (for reading purposes).

## Definition

```c
void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len, size_t clen);
```

## Related

[RvR_rw_init_const_mem](/rvr/rvr/rw_init_const_mem)

[RvR_rw_init_dyn_mem](/rvr/rvr/rw_init_dyn_mem)
