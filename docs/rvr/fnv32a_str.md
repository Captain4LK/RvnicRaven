# RvR_fnv32a_str

Function for calculating the hash of a string using the fnv32a hash. Additionally you can specify the initial value of the hash (hval), this can be used to chain hashes.

## Definition

```c
uint32_t RvR_fnv32a_str(const char *str, uint32_t hval);
```

## Related

[RvR_fnv64a_str](/rvr/rvr/fnv64a_str)

[RvR_fnv32a](/rvr/rvr/fnv32a)

[RvR_fnv32a_buf](/rvr/rvr/fnv32a_buf)

