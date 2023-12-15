# RvR_texture_create

Function for manually creating a texture. Textures created likes this need to be manually freed and will stay availible between RvR_texture_get() calls.

## Definition

```c
void RvR_texture_create(uint16_t id, int width, int height);
```

## Related

[RvR_texture](texture.md)

[RvR_texture_get](texture_get.md)

[RvR_texture_create_free](texture_create_free.md)
