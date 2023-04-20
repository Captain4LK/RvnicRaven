# RvR_realloc

Tries to resize an allocation to the specified amount of memory. If RvR_malloc_init() has'nt been called yet, RvR_realloc() will call libc realloc(). Additionall you need to provide a reason for the allocation, which will be shown by RvR_malloc_report().

## Definition

```c
void *RvR_realloc(void *ptr, size_t size, const char *reason);
```

## Related

[RvR_malloc](/rvr/rvr/malloc)

[RvR_free](/rvr/rvr/free)
