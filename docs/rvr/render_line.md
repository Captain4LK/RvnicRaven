# RvR_render_line

Renders the line specified by the 24.8 fixed point coordinates. If you want to draw the line at full pixel coordinates, make sure the start and end coordinates go through the middle of the pixel (+128).

## Definition

```c
void RvR_render_line(RvR_fix24 x0, RvR_fix24 y0, RvR_fix24 x1, RvR_fix24 y1, uint8_t index);
```

## Related

[RvR_render_vertical_line](/rvr/rvr/render_vertical_line)

[RvR_render_horizontal_line](/rvr/rvr/render_horizontal_line)
