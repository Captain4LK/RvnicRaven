# RvR_fnv32a_buf

Function for calculating the hash of an arbitrary buffer using the fnv32a hash. Additionally you can specify the initial value of the hash (hval), this can be used to chain hashes.

## Definition

```c
uint32_t RvR_fnv32a_buf(const void *buf, size_t len, uint32_t hval);
```

## Related

[RvR_fnv64a_buf](/rvr/rvr/fnv64a_buf)

[RvR_fnv32a](/rvr/rvr/fnv32a)

[RvR_fnv32a_str](/rvr/rvr/fnv32a_str)
