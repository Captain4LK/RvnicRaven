# RvR_rw_usr_write

Function that gets called when running RvR_rw_write() for a RVR_RW_USR RvR_rw struct.

## Definition

```c
typedef size_t (*RvR_rw_usr_write) (RvR_rw *rw, const void *buffer, size_t size, size_t count);
```

## Related functions

[RvR_rw_write](/rvr/rvr/rw_write)

[RvR_rw](/rvr/rvr/rw)
