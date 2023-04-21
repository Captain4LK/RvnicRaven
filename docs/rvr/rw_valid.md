# RvR_rw_valid

Checks the rw.type field for type validity. If you always zero-initialize your RvR_rw struct, you can detect uninitialized (or closed) RvR_rw struct with this function.

## Definition

```c
int RvR_rw_valid(const RvR_rw *rw);
```

## Related

[RvR_rw](/rvr/rvr/rw)

[RvR_rw_type](/rvr/rvr/rw_type)
