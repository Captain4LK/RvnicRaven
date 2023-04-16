# RvR_rand_well

State for well (Well Equidistributed Long-period Linear) random number generator. Here is a page containing some information about well generators: [wikipedia](https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear).

## Definition

```c
typedef uint32_t RvR_rand_pcg[17];
```

## Related functions

[RvR_rand_well_seed(RvR_rand_well *well, uint32_t seed);](/rvr/rvr/rand_well_seed)

[RvR_rand_well_next(RvR_rand_well *well);](/rvr/rvr/rand_well_next)

[RvR_rand_well_next_range(RvR_rand_well *well, int32_t min, int32_t max);](/rvr/rvr/rand_well_next_range)
