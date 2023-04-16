# RvR_non_zero

Macro for making sure a number isn't zero. If a is 0, it get's set to 1, useful for prevent div by zero errors during mathematical edge cases.

## Definition

```c
#define RvR_non_zero(a) (...)
```

## Notes

Since this is implemented as a macro, arguments may be expanded more than once.
