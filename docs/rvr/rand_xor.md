# RvR_rand_xor

State for xor (Xorshift) random number generator. More information about the generator can be found here: [wikipedia](https://en.wikipedia.org/wiki/Xorshift).

## Definition

```c
typedef uint64_t RvR_rand_xor[2];
```

## Related functions

[RvR_rand_xor_seed(RvR_rand_xor *xor, uint64_t seed);](/rvr/rvr/rand_xor_seed)

[RvR_rand_xor_next(RvR_rand_xor *xor);](/rvr/rvr/rand_xor_next)

[RvR_rand_xor_next_range(RvR_rand_xor *xor, int32_t min, int32_t max);](/rvr/rvr/rand_xor_next_range)
