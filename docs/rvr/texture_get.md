# RvR_texture_get

Tries to load the texture from the ("TEX%05d",id) lump. Returned pointer is only guaranteed to be valid until the next RvR_texture_get() call.

## Definition

```c
RvR_texture *RvR_texture_get(uint16_t id);
```

## Related

[RvR_texture](texture.md)

[RvR_texture_create](texture_create.md)
