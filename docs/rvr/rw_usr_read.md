# RvR_rw_usr_read

Function that gets called when running RvR_rw_read() for a RVR_RW_USR RvR_rw struct.

## Definition

```c
typedef size_t (*RvR_rw_usr_read)  (RvR_rw *rw, void *buffer, size_t size, size_t count);
```

## Related functions

[RvR_rw_read](/rvr/rvr/rw_read)

[RvR_rw](/rvr/rvr/rw)
