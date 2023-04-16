# RvR_rand_pcg

State for pcg random number generator. Here is a page containing some information about pcg generators: [pcg-random](https://www.pcg-random.org/).

## Definition

```c
typedef uint64_t RvR_rand_pcg[2];
```

## Related functions

[RvR_rand_pcg_seed(RvR_rand_pcg *pcg, uint32_t seed);](/rvr/rvr/rand_pcg_seed)

[RvR_rand_pcg_next(RvR_rand_pcg *pcg);](/rvr/rvr/rand_pcg_next)

[RvR_rand_pcg_next_range(RvR_rand_pcg *pcg, int32_t min, int32_t max);](/rvr/rvr/rand_pcg_next_range)
