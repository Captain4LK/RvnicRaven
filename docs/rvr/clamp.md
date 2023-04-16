# RvR_clamp

Macro for clamping a number between min and max.

## Definition

```c
#define RvR_clamp(a,min,max) (...)
```

## Notes

Since this is implemented as a macro, arguments may be expanded more than once. Additionally, RvR_clamp expects min>=max.

## Related

[RvR_min](/rvr/rvr/min)

[RvR_max](/rvr/rvr/max)
