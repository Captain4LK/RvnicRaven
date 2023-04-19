# RvR_fnv64a_buf

Function for calculating the hash of an arbitrary buffer using the fnv64a hash. Additionally you can specify the initial value of the hash (hval), this can be used to chain hashes.

## Definition

```c
uint64_t RvR_fnv64a_buf(const void *buf, size_t len, uint64_t hval);
```

## Related

[RvR_fnv32a_buf](/rvr/rvr/fnv32a_buf)

[RvR_fnv64a](/rvr/rvr/fnv64a)

[RvR_fnv64a_str](/rvr/rvr/fnv64a_str)
