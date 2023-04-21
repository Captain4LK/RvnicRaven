# RvR_shade_table

Returns an array of 256 entries used for mapping colors to the correct light level. 

Level 0: original colors

Level 63 (max): black

## Definition

```c
uint8_t *RvR_shade_table(uint8_t light);
```

## Related

[RvR_palette_load](/rvr/rvr/palette_load)

[RvR_color](/rvr/rvr/color)

[RvR_blend](/rvr/rvr/blend)

[RvR_palette](/rvr/rvr/palette)
