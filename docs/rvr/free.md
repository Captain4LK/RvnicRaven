# RvR_free

Frees memory previously allocated by RvR_malloc(). If RvR_malloc_init() hasn't been called yet, RvR_free() will call libc free().

## Definition

```c
void RvR_free(void *ptr);
```

## Related

[RvR_malloc](/rvr/rvr/malloc)

[RvR_realloc](/rvr/rvr/realloc)
