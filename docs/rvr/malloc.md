# RvR_malloc

Tries to allocate the specified amount of memory. If RvR_malloc_init() has'nt been called yet, RvR_malloc() will call libc malloc(). Additionall you need to provide a reason for the allocation, which will be shown by RvR_malloc_report().

## Definition

```c
void *RvR_malloc(size_t size, const char *reason);
```

## Related

[RvR_malloc_init](/rvr/rvr/malloc_init)

[RvR_malloc_report](/rvr/rvr/malloc_report)

[RvR_realloc](/rvr/rvr/realloc)

[RvR_free](/rvr/rvr/free)
